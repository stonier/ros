/*
 * Copyright (C) 2009, Willow Garage, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the names of Stanford University or Willow Garage, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ROSCPP_SUBSCRIPTION_CALLBACK_HELPER_H
#define ROSCPP_SUBSCRIPTION_CALLBACK_HELPER_H

#include <typeinfo>

#include "ros/forwards.h"
#include "ros/message_traits.h"
#include "ros/builtin_message_traits.h"
#include "ros/serialization.h"
#include "ros/message.h"
#include "ros/message_event.h"
#include <ros/static_assert.h>

#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/make_shared.hpp>

namespace ros
{

template<typename M>
inline boost::shared_ptr<M> defaultMessageCreateFunction()
{
  return boost::make_shared<M>();
}

template<typename T>
typename boost::enable_if<boost::is_base_of<ros::Message, T> >::type assignSubscriptionConnectionHeader(T* t, const boost::shared_ptr<M_string>& connection_header)
{
  t->__connection_header = connection_header;
}

template<typename T>
typename boost::disable_if<boost::is_base_of<ros::Message, T> >::type assignSubscriptionConnectionHeader(T* t, const boost::shared_ptr<M_string>& connection_header)
{
}

struct SubscriptionCallbackHelperDeserializeParams
{
  uint8_t* buffer;
  uint32_t length;
  boost::shared_ptr<M_string> connection_header;
};

struct SubscriptionCallbackHelperCallParams
{
  MessageEvent<void const> event;
};

/**
 * \brief Internal use
 *
 * The SubscriptionCallbackAdapter is templated on the callback parameter type (\b not the bare message type), and provides 3 things:
 *  - MessageType, which provides the bare message type, no const or reference qualifiers
 *  - CallbackType, which provides the full Boost.Function callback type
 *  - the static call() method, which calls the callback in the correct manner given the callback type
 *
 *  SubscriptionCallbackAdapter is then specialized to allow callbacks of any of the forms:
\verbatim
void callback(const boost::shared_ptr<M const>&);
void callback(boost::shared_ptr<M const>);
void callback(const M&);
void callback(M);
void callback(const MessageEvent<M const>&);
\endverbatim
 */
template<typename M>
struct SubscriptionCallbackAdapter
{
  typedef typename boost::remove_reference<typename boost::remove_const<M>::type>::type MessageType;
  typedef boost::function<void(MessageType)> CallbackType;
  typedef MessageEvent<MessageType const> EventType;

  static const bool is_const = true;

  static void call(const CallbackType& cb, const EventType& event)
  {
    cb(*event.getMessage());
  }
};

/**
 * Callback type void callback(const M&);
 */
template<typename M>
struct SubscriptionCallbackAdapter<const M&>
{
  typedef typename boost::remove_reference<typename boost::remove_const<M>::type>::type MessageType;
  typedef boost::function<void(const MessageType&)> CallbackType;
  typedef MessageEvent<MessageType const> EventType;

  static const bool is_const = true;

  static void call(const CallbackType& cb, const EventType& event)
  {
    cb(*event.getMessage());
  }
};

/**
 * Callback type void callback(const boost::shared_ptr<M>&);.  Use of non-const messages is not implemented yet,
 * so anything that would instantiate this will error at compile time
 */
template<typename M>
struct SubscriptionCallbackAdapter<const boost::shared_ptr<M>& >
{
  typedef typename boost::remove_reference<typename boost::remove_const<M>::type>::type MessageType;
  typedef boost::function<void(const boost::shared_ptr<MessageType>&)> CallbackType;
  typedef MessageEvent<MessageType> EventType;

  static const bool is_const = false;

  static void call(const CallbackType& cb, const EventType& event)
  {
    cb(event.getMessage());
  }
};

/**
 * Callback type void callback(const boost::shared_ptr<M const>&);
 */
template<typename M>
struct SubscriptionCallbackAdapter<const boost::shared_ptr<M const>& >
{
  typedef typename boost::remove_reference<typename boost::remove_const<M>::type>::type MessageType;
  typedef boost::function<void(const boost::shared_ptr<MessageType const>&)> CallbackType;
  typedef MessageEvent<MessageType const> EventType;

  static const bool is_const = true;

  static void call(const CallbackType& cb, const EventType& event)
  {
    cb(event.getMessage());
  }
};


/**
 * Callback type void callback(boost::shared_ptr<M const>);
 */
template<typename M>
struct SubscriptionCallbackAdapter<boost::shared_ptr<M const> >
{
  typedef typename boost::remove_reference<typename boost::remove_const<M>::type>::type MessageType;
  typedef boost::function<void(boost::shared_ptr<MessageType const>)> CallbackType;
  typedef MessageEvent<MessageType const> EventType;

  static const bool is_const = true;

  static void call(const CallbackType& cb, const EventType& event)
  {
    cb(event.getMessage());
  }
};

/**
 * Callback type void callback(const boost::shared_ptr<M const>&);.  Use of non-const messages is not implemented yet,
 * so anything that would instantiate this will error at compile time
 */
template<typename M>
struct SubscriptionCallbackAdapter<boost::shared_ptr<M> >
{
  typedef typename boost::remove_reference<typename boost::remove_const<M>::type>::type MessageType;
  typedef boost::function<void(boost::shared_ptr<MessageType>)> CallbackType;
  typedef MessageEvent<MessageType> EventType;

  static const bool is_const = false;

  static void call(const CallbackType& cb, const EventType& event)
  {
    cb(event.getMessage());
  }
};

/**
 * Callback type void callback(const ros::MessageEvent<M>&);.  Use of non-const messages is not implemented yet,
 * so anything that would instantiate this will error at compile time
 */
template<typename M>
struct SubscriptionCallbackAdapter<const MessageEvent<M>& >
{
  typedef typename boost::remove_reference<typename boost::remove_const<M>::type>::type MessageType;
  typedef boost::function<void(const MessageEvent<MessageType>&)> CallbackType;
  typedef MessageEvent<MessageType> EventType;

  static const bool is_const = false;

  static void call(const CallbackType& cb, const EventType& event)
  {
    cb(event);
  }
};

/**
 * Callback type void callback(const ros::MessageEvent<M const>&);
 */
template<typename M>
struct SubscriptionCallbackAdapter<const MessageEvent<M const>& >
{
  typedef typename boost::remove_reference<typename boost::remove_const<M>::type>::type MessageType;
  typedef boost::function<void(const MessageEvent<MessageType const>&)> CallbackType;
  typedef MessageEvent<MessageType const> EventType;

  static const bool is_const = true;

  static void call(const CallbackType& cb, const EventType& event)
  {
    cb(event);
  }
};

/**
 * \brief Abstract base class used by subscriptions to deal with concrete message types through a common
 * interface.  This is one part of the roscpp API that is \b not fully stable, so overloading this class
 * is not recommended.
 */
class SubscriptionCallbackHelper
{
public:
  virtual ~SubscriptionCallbackHelper() {}
  virtual VoidConstPtr deserialize(const SubscriptionCallbackHelperDeserializeParams&) = 0;
  virtual void call(SubscriptionCallbackHelperCallParams& params) = 0;
  virtual const std::type_info& getTypeInfo() = 0;
  virtual bool isConst() = 0;
};
typedef boost::shared_ptr<SubscriptionCallbackHelper> SubscriptionCallbackHelperPtr;

/**
 * \brief Concrete generic implementation of SubscriptionCallbackHelper for any normal message type.  Use directly with
 * care, this is mostly for internal use.
 */
template<typename M, typename Enabled = void>
class SubscriptionCallbackHelperT : public SubscriptionCallbackHelper
{
public:
  typedef typename SubscriptionCallbackAdapter<M>::MessageType NonConstType;
  typedef typename SubscriptionCallbackAdapter<M>::EventType EventType;
  typedef typename boost::add_const<NonConstType>::type ConstType;
  typedef boost::shared_ptr<NonConstType> NonConstTypePtr;
  typedef boost::shared_ptr<ConstType> ConstTypePtr;

  static const bool is_const = SubscriptionCallbackAdapter<M>::is_const;

  typedef typename SubscriptionCallbackAdapter<M>::CallbackType Callback;
  typedef boost::function<NonConstTypePtr()> CreateFunction;

  SubscriptionCallbackHelperT(const Callback& callback, const CreateFunction& create = defaultMessageCreateFunction<NonConstType>)
  : callback_(callback)
  , create_(create)
  {}

  void setCreateFunction(const CreateFunction& create)
  {
    create_ = create;
  }

  virtual VoidConstPtr deserialize(const SubscriptionCallbackHelperDeserializeParams& params)
  {
    namespace ser = serialization;

    NonConstTypePtr msg = create_();
    assignSubscriptionConnectionHeader(msg.get(), params.connection_header);

    ser::IStream stream(params.buffer, params.length);
    ser::deserialize(stream, *msg);

    return VoidConstPtr(msg);
  }

  virtual void call(SubscriptionCallbackHelperCallParams& params)
  {
    EventType event(params.event, create_);
    SubscriptionCallbackAdapter<M>::call(callback_, event);
  }

  virtual const std::type_info& getTypeInfo()
  {
    return typeid(NonConstType);
  }

  virtual bool isConst()
  {
    return SubscriptionCallbackAdapter<M>::is_const;
  }

private:
  Callback callback_;
  CreateFunction create_;
};

}

#endif // ROSCPP_SUBSCRIPTION_CALLBACK_HELPER_H
