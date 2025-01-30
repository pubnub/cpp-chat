#ifndef PN_CHAT_CALLBACK_HANDLE_HPP
#define PN_CHAT_CALLBACK_HANDLE_HPP

#include <memory>

class Subscribable;

namespace Pubnub {
    class CallbackHandle {
        public: 
            CallbackHandle(std::shared_ptr<Subscribable> subscription);
            ~CallbackHandle() = default;

            void close();

        private:
            std::shared_ptr<Subscribable> subscription;
    };
}

#endif // PN_CHAT_CALLBACK_HANDLE_HPP
