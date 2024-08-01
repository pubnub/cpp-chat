#include "c_thread_message.hpp"
#include "c_errors.hpp"

void pn_thread_message_dispose(Pubnub::ThreadMessage* thread_message){
    delete thread_message;
}