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
    InvalidHandler
  };

  std::ostream & operator << (std::ostream & out, ChannelError outputError) {
    if (outputError == ChannelError::Unknown) {
      out << "ChannelError [unknown]";
    } else if (outputError == ChannelError::BrokenPipe) {
      out << "ChannelError [broken pipe]";
    } else if (outputError == ChannelError::NoData) {
      out << "ChannelError [unavailable data]";
    } else if (outputError == ChannelError::InvalidHandler) {
      out << "ChannelError [invalid handler]";
    }

    return out;
  }

  class Handler final {

    public:
      Handler(int fd) {
        mDescriptor = fd;
      }

      int get_value() const noexcept {
        return mDescriptor;
      }

      operator int () const noexcept {
        return mDescriptor;
      }

      bool operator == (Handler const & rhs) const noexcept {
        return get_value() == rhs.get_value();
      }

    private:
      int mDescriptor;

  };

  template <typename ...Args>
    class Channel;

  namespace details {

    template <typename Channel>
    struct StreamMode {

      public:
        StreamMode(std::unique_ptr<Channel> &channel):
          mChannel{channel} {
        }

        std::unique_ptr<Channel> & get_channel() {
          return mChannel;
        }

        Handler get_handler() {
          return mChannel->get_handler();
        }

      private:
        std::unique_ptr<Channel> &mChannel;

    };

    class Input final {

      template <typename ...Args>
      friend class jchannel::Channel;

      public:
        ~Input() {
          close();
        }

        Handler const & get_handler() const noexcept {
          return mHandler;
        }

        bool packet_mode_enabled() {
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
              // timeout: return error
            }

            if (r < 0) {
              // error:: return error
            }

            return read<T>();
          }

        template <typename T = Empty, class Clock, class Duration>
          tl::expected<T, ChannelError> read_until(std::chrono::time_point<Clock, Duration> const &timeout_time) {
            using namespace std::chrono;

            read_for(timeout_time - decltype(timeout_time)::now());
          }

        void close() const noexcept {
          ::close(mHandler);
        }

      private:
        Handler mHandler;
        bool mPacketMode {false};

        Input(int fd, bool packetMode = false):
          mHandler{fd}, mPacketMode{packetMode} {
        }

    };

    class Output final {

      template <typename ...Args>
      friend class jchannel::Channel;

      public:
        ~Output() {
          close();
        }

        Handler const & get_handler() const noexcept {
          return mHandler;
        }

        bool packet_mode_enabled() {
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
              // timeout: return error
            }

            if (r < 0) {
              // error:: return error
            }

            return write(std::forward<T>(value));
          }

        template <typename T = Empty, class Clock, class Duration>
          tl::expected<ssize_t, ChannelError> write_until(std::chrono::time_point<Clock, Duration> const &timeout_time) {
            using namespace std::chrono;

            return write_for(timeout_time - decltype(timeout_time)::now());
          }

        void close() const noexcept {
          ::close(mHandler);
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
      bool mPacketMode {0};

      template <std::size_t Index>
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

          _process_parameters<0>(t);

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
          Polling(F & callback, Args && ...args):
            mCallback{callback}, mArgs{std::move(args)...}
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

            try {
              std::apply(
                  [&]<typename ...fArgs>(fArgs & ...args) {
                    (create_channel_poll(args), ...);
                  }, mArgs);
            } catch (...) {
              throw;
            }
          }

          ~Polling() {
            ::close(mEpoll);
          }

          Polling & operator () () {
            int n;

            do {
              n = epoll_wait(mEpoll, mEvents, ArgsSize, -1);
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
                  [&]<typename ...fArgs>(fArgs && ...args) {
                    for_each(fd, mCallback, std::forward<fArgs>(args)...);
                  }, mArgs);
            }

            return *this;
          }

        private:
          struct CallbackRedirect {

            F const  & mCallback;

            void operator() (auto & arg) {
              std::invoke(mCallback, std::forward<decltype(arg)>(arg));
            }

            template <typename Channel>
              void operator() (std::unique_ptr<details::StreamMode<Channel>> & arg) {
                std::invoke(mCallback, std::forward<decltype(arg)>(arg)->get_channel());
              }

          };

          constexpr static const int ArgsSize = sizeof...(Args);

          std::tuple<Args ...> mArgs;
          CallbackRedirect const  & mCallback;
          int mEpoll {-1};
          struct epoll_event mEvents[sizeof...(Args)];

          void register_polling(struct epoll_event & event) {
            if (epoll_ctl(mEpoll, EPOLL_CTL_ADD, event.data.fd, &event) == -1) {
              throw std::runtime_error("unable to polling handler");
            }
          }

          void create_channel_poll(std::unique_ptr<details::Input> & value) {
            struct epoll_event e;
            bool packet {false};

            e.events = EPOLLIN;

            if (value->packet_mode_enabled() == true) {
              e.events = e.events | EPOLLET;
            }

            e.data.fd = value->get_handler().get_value();

            register_polling(e);
          }

          void create_channel_poll(std::unique_ptr<details::StreamMode<details::Input>> & value) {
            struct epoll_event e;

            e.events = EPOLLIN;
            e.data.fd = value->get_handler();

            register_polling(e);
          }

          void create_channel_poll(std::unique_ptr<details::Output> & value) {
            struct epoll_event e;
            bool packet {false};

            e.events = EPOLLOUT;

            if (value->packet_mode_enabled() == true) {
              e.events = e.events | EPOLLET;
            }

            e.data.fd = value->get_handler().get_value();

            register_polling(e);
          }

          void create_channel_poll(std::unique_ptr<details::StreamMode<details::Output>> & value) {
            struct epoll_event e;

            e.events = EPOLLOUT;
            e.data.fd = value->get_handler();

            register_polling(e);
          }

      };

  }

  using Input = std::unique_ptr<details::Input>;
  using Output = std::unique_ptr<details::Output>;

  template <typename F, typename ...Args>
    auto poll(F callback, Args && ...args) {
      return details::Polling{callback, std::move(args)...};
    }

  template <typename Channel>
    auto as_stream(std::unique_ptr<Channel> & channel) {
      return std::make_unique<details::StreamMode<Channel>>(channel);
    }

}

