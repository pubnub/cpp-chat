#ifndef PN_CHAT_CALLBACK_HANDLE_HPP
#define PN_CHAT_CALLBACK_HANDLE_HPP

#include <memory>

class Subscription;

namespace Pubnub {
    class CallbackHandle {
        public: 
            CallbackHandle(std::shared_ptr<Subscription> subscription);
            ~CallbackHandle() = default;

            void close();

        private:
            std::shared_ptr<Subscription> subscription;
    };
}

#endif // PN_CHAT_CALLBACK_HANDLE_HPP
