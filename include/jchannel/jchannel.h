#include <iostream>
#include <optional>
#include <memory>
#include <thread>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include "expected.hpp"

namespace jchannel {

  class alignas(1) Empty {
  };

  // INFO:: channel paramenters
  struct NonBlocking {};
  struct CloseOnExec {};
  struct PacketMode {};

  enum class ChannelError {
    Unknown,
    BrokenPipe,
    NoData,
    InvalidHandler,
    Timeout
  };

  std::ostream & operator << (std::ostream & out, Empty outputError) {
    out << "[]";

    return out;
  }

  std::ostream & operator << (std::ostream & out, ChannelError outputError) {
    if (outputError == ChannelError::Unknown) {
      out << "[Unknown Error]";
    } else if (outputError == ChannelError::BrokenPipe) {
      out << "[Broken Pipe]";
    } else if (outputError == ChannelError::NoData) {
      out << "[Unavailable Data]";
    } else if (outputError == ChannelError::InvalidHandler) {
      out << "[Invalid Handler]";
    }

    return out;
  }

  class Handler final {

    public:
      Handler(int fd) {
        mDescriptor = fd;
      }

      Handler(Handler const & handler):
        mDescriptor{handler.mDescriptor} {
      }

      Handler(Handler && handler):
        mDescriptor{handler.mDescriptor} {
          handler.mDescriptor = -1;
      }

      ~Handler() {
      }

      operator int () const noexcept {
        return get_value();
      }

      int get_value() const noexcept {
        return mDescriptor;
      }

      bool duplicate(int fd) const noexcept {
        int flags = fcntl(get_value(), F_GETFD);

        if (flags < 0) {
          return false;
        }

        return dup3(get_value(), fd, (flags & FD_CLOEXEC)?O_CLOEXEC:0) == fd;
      }

      void close() const noexcept {
        ::close(get_value());
      }

      bool operator == (Handler const & rhs) const noexcept {
        return get_value() == rhs.get_value();
      }

    private:
      int mDescriptor;

  };

  class IStream {

    public:
      virtual ~IStream() {
      }

      virtual Handler const & get_handler() const noexcept = 0;

      virtual bool packet_mode_enabled() const noexcept = 0;

  };

  class IInput : public virtual IStream {

    public:
      virtual ~IInput() {
      }

  };

  class IOutput : public virtual IStream {

    public:
      virtual ~IOutput() {
      }

  };

  template <typename ...Args>
    class Channel;

  namespace details {

    class Input : public IInput {

      template <typename ...Args>
      friend class jchannel::Channel;

      public:
        ~Input() {
          mHandler.close();
        }

        Handler const & get_handler() const noexcept {
          return mHandler;
        }

        bool packet_mode_enabled() const noexcept {
          return mPacketMode;
        }

        template <typename T = Empty>
          tl::expected<T, ChannelError> read() {
          // [[nodiscard]] expected<T, ChannelError> read() {
            T value;
            int r;

            do {
              r = ::read(mHandler, &value, sizeof(value));
            } while (r < 0 and errno == EINTR);

            if (r == 0) {
              return tl::unexpected{ChannelError::BrokenPipe};
            } else if (r < 0) {
              if (errno == EAGAIN or errno == EWOULDBLOCK) {
                return tl::unexpected{ChannelError::NoData};
              } else if (errno == EBADF) {
                return tl::unexpected{ChannelError::InvalidHandler};
              }

              return tl::unexpected{ChannelError::Unknown};
            }

            return value;
          }

        template <typename T = Empty, class Rep, class Period>
          tl::expected<T, ChannelError> read_for(std::chrono::duration<Rep, Period> rel_time) {
            using namespace std::chrono;

            // [[nodiscard]] expected<T, ChannelError> read() {
            // return Err{ChannelError::BrokenPipe};
            struct timeval timeout;
            fd_set fd;

            FD_ZERO(&fd);
            FD_SET(mHandler, &fd);

            seconds secs = seconds{rel_time};
            rel_time -= secs;
            microseconds usecs = microseconds{rel_time};

            timeout.tv_sec = secs.count();
            timeout.tv_usec = usecs.count();

            int r;

            do {
              r = ::select(mHandler + 1, &fd, nullptr, nullptr, &timeout);
            } while (r < 0 and errno == EINTR);

            if (r == 0) {
              return tl::unexpected{ChannelError::Timeout};
            }

            if (r < 0) {
              return tl::unexpected{ChannelError::Unknown};
            }

            return read<T>();
          }

        template <typename T = Empty, class Clock, class Duration>
          tl::expected<T, ChannelError> read_until(std::chrono::time_point<Clock, Duration> const &timeout_time) {
            using namespace std::chrono;

            read_for(timeout_time - decltype(timeout_time)::now());
          }

        void close() const noexcept {
          mHandler.close();
        }

      private:
        Handler mHandler;
        bool mPacketMode {false};

        Input(int fd, bool packetMode = false):
          mHandler{fd}, mPacketMode{packetMode} {
        }

    };

    class Output : public IOutput {

      template <typename ...Args>
      friend class jchannel::Channel;

      public:
        ~Output() {
          mHandler.close();
        }

        Handler const & get_handler() const noexcept {
          return mHandler;
        }

        bool packet_mode_enabled() const noexcept {
          return mPacketMode;
        }

        template <typename T = Empty>
          tl::expected<ssize_t, ChannelError> write(T && value = T {}) {
            if (mHandler.get_value() <= 0) {
              return -1;
            }

            ssize_t r;

            do {
              r = ::write(mHandler, &value, sizeof(value));
            } while (r < 0 and errno == EINTR);

            if (r < 0) {
              if (errno == EPIPE) {
                return tl::unexpected{ChannelError::BrokenPipe};
              } else {
                return tl::unexpected{ChannelError::Unknown};
              }
            }

            return r;
          }

        template <typename T = Empty, class Rep, class Period>
          tl::expected<ssize_t, ChannelError> write_for(T && value, std::chrono::duration<Rep, Period> rel_time) {
            using namespace std::chrono;

            // [[nodiscard]] expected<T, ChannelError> read() {
            // return Err{ChannelError::BrokenPipe};
            struct timeval timeout;
            fd_set fd;

            FD_ZERO(&fd);
            FD_SET(mHandler, &fd);

            seconds secs = seconds{rel_time};
            rel_time -= secs;
            microseconds usecs = microseconds{rel_time};

            timeout.tv_sec = secs.count();
            timeout.tv_usec = usecs.count();

            int r;

            do {
              r = ::select(mHandler + 1, nullptr, &fd, nullptr, &timeout);
            } while (r < 0 and errno == EINTR);

            if (r == 0) {
              return tl::unexpected{ChannelError::Timeout};
            }

            if (r < 0) {
              return tl::unexpected{ChannelError::Unknown};
            }

            return write(std::forward<T>(value));
          }

        template <typename T = Empty, class Clock, class Duration>
          tl::expected<ssize_t, ChannelError> write_until(std::chrono::time_point<Clock, Duration> const &timeout_time) {
            using namespace std::chrono;

            return write_for(timeout_time - decltype(timeout_time)::now());
          }

        void close() const noexcept {
          mHandler.close();
        }

      private:
        Handler mHandler;
        bool mPacketMode {false};

        Output(int fd, bool packetMode = false):
          mHandler{fd}, mPacketMode{packetMode} {
        }

    };

  }

  template<typename ...Params>
    class Channel {

      int mCloseOnExec {0};
      int mNonBlocking {0};
      int mPacketMode {0};

      template <std::size_t Index = 0>
        void _process_parameters(std::tuple<Empty, Params...> t)
        {
          using TupleType = decltype(t);

          if constexpr (Index >= std::tuple_size<TupleType>()) {
            return;
          }

          using IndexType = std::tuple_element_t<Index, TupleType>;

          if (std::is_same<IndexType, CloseOnExec>::value == true) {
            mCloseOnExec = O_CLOEXEC;
          } else if (std::is_same<IndexType, NonBlocking>::value == true) {
            mNonBlocking = O_NONBLOCK;
          } else if (std::is_same<IndexType, PacketMode>::value == true) {
            mPacketMode = O_DIRECT;
          }

          if constexpr (Index < std::tuple_size<TupleType>() - 1) {
            _process_parameters<Index + 1>(t);
          }
        }

      public:
        Channel()
        {
          std::tuple<Empty, Params...> t;

          _process_parameters(t);

          int fd[2];

          if (pipe2(fd, mNonBlocking | mCloseOnExec | mPacketMode) != 0) {
            throw std::runtime_error("unable to create channel");
          }

          details::Input *in = new details::Input{fd[0]};
          details::Output *out = new details::Output{fd[1]};

          mInput = std::unique_ptr<details::Input>(in);
          mOutput = std::unique_ptr<details::Output>(out);
        }

        [[nodiscard]] std::unique_ptr<details::Input> get_input() {
          return std::move(mInput);
        }

        [[nodiscard]] std::unique_ptr<details::Output> get_output() {
          return std::move(mOutput);
        }

        [[nodiscard]] auto get_channels() {
          struct result {
            std::unique_ptr<details::Input> input;
            std::unique_ptr<details::Output> output;
          };

          return result{std::move(mInput), std::move(mOutput)};
        }

      private:
        std::unique_ptr<details::Input> mInput;
        std::unique_ptr<details::Output> mOutput;

    };

  namespace details {

    template <typename F, typename Arg, typename ...Args>
      static void for_each(int handler, F callback, Arg & arg, Args & ...args) {
        if (arg->get_handler() == Handler{handler}) {
          std::invoke(callback, arg);
        } else {
          if constexpr (sizeof...(args) > 0) {
            return for_each(handler, callback, args...);
          }
        }
      }
            
    template <typename F, typename ...Args>
      class Polling {

        public:
          Polling(F & callback, Args & ...args):
            mCallback{callback}, mArgs{args...}
          {
            mEpoll = epoll_create1(EPOLL_CLOEXEC);

            if (mEpoll < 0) {
              if (errno == EMFILE) {
                throw std::out_of_range("poll instances limit was reached");
              } else if (errno == ENFILE) {
                throw std::out_of_range("descriptor instances limit was reached");
              } else if (errno == ENOMEM) {
                throw std::runtime_error("no available memory");
              }
            }

            init_close_pipe();

            try {
              std::apply(
                  [&]<typename ...fArgs>(fArgs & ...args) {
                    (create_channel_poll(args.get()), ...);
                  }, mArgs);
            } catch (...) {
              throw;
            }
          }

          ~Polling() {
            close();
            
            release_close_pipe();
          }

          operator bool() {
            return mEpoll >= 0;
          }

          Polling & operator () () {
            int n;

            do {
              n = epoll_wait(mEpoll, mEvents, ArgsSize, -1);
            } while (n < 0 and errno == EINTR);

            if (n < 0) {
              if (mEpoll < 0) {
                return *this;
              }

              throw std::runtime_error("invalid descriptor");
            }

            for (int i = 0; i < n; i++) {
              int fd = mEvents[i].data.fd;

              std::apply(
                  [&]<typename ...fArgs>(fArgs & ...args) {
                    for_each(fd, mCallback, args...);
                  }, mArgs);
            }

            return *this;
          }
          
          Polling & operator () (std::chrono::milliseconds ms) {
            int n;

            do {
              n = epoll_wait(mEpoll, mEvents, ArgsSize, ms.count());
            } while (n < 0 and errno == EINTR);

            if (n < 0) {
              throw std::runtime_error("invalid descriptor");
            }

            for (int i = 0; i < n; i++) {
              int fd = mEvents[i].data.fd;

              std::apply(
                  [&]<typename ...fArgs>(fArgs & ...args) {
                    for_each(fd, mCallback, args...);
                  }, mArgs);
            }

            return *this;
          }

          void close() {
            if (mEpoll > 0) {
              char ch = 1;

              ::close(mEpoll);
              ::write(mClosePipe[1], &ch, 1);
            }

            mEpoll = -1;
          }

        private:
          struct CallbackRedirect {

            F mCallback;

            void operator() (auto & arg) {
              std::invoke(mCallback, arg);
            }

          };

          constexpr static const int ArgsSize = sizeof...(Args);

          epoll_event mEvents[sizeof...(Args)];
          std::tuple<Args & ...> mArgs;
          CallbackRedirect mCallback;
          int mClosePipe[2];
          int mEpoll {-1};

          void init_close_pipe() {
            if (pipe2(mClosePipe, O_CLOEXEC) < 0) {
              throw std::runtime_error("unable to create close handled");
            }

            struct epoll_event e;

            e.events = EPOLLIN;
            e.events = e.events | EPOLLET;
            e.data.fd = mClosePipe[0];

            register_polling(e);
          }

          void release_close_pipe() {
            ::close(mClosePipe[0]);
            ::close(mClosePipe[1]);
          }

          void register_polling(struct epoll_event & event) {
            if (epoll_ctl(mEpoll, EPOLL_CTL_ADD, event.data.fd, &event) == -1) {
              throw std::runtime_error("unable to polling handler");
            }
          }

          void create_channel_poll(IInput * value) {
            struct epoll_event e;

            e.events = EPOLLIN;

            if (value->packet_mode_enabled() == true) {
              e.events = e.events | EPOLLET;
            }

            e.data.fd = value->get_handler().get_value();

            register_polling(e);
          }

          void create_channel_poll(IOutput * value) {
            struct epoll_event e;

            e.events = EPOLLOUT;

            if (value->packet_mode_enabled() == true) {
              e.events = e.events | EPOLLET;
            }

            e.data.fd = value->get_handler().get_value();

            register_polling(e);
          }

      };

  }

  using Input = std::unique_ptr<details::Input>;
  using Output = std::unique_ptr<details::Output>;

  template <typename F, typename ...Args>
    auto poll(F callback, Args & ...args) {
      return details::Polling{callback, args...};
    }

}

