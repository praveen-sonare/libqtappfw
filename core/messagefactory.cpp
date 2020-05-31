/*
 * Copyright (C) 2020 Konsulko Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "messagefactory.h"
#include "message.h"
#include "responsemessage.h"
#include "eventmessage.h"
#include "callmessage.h"

std::unique_ptr<Message> MessageFactory::createInboundMessage(MessageId id, QJsonDocument data)
{
	std::unique_ptr<Message> msg(nullptr);
	if ((id == MessageId::RetOk) || (id == MessageId::RetErr))
		msg.reset(new ResponseMessage(data));
	else if (id == MessageId::Event)
		msg.reset(new EventMessage(data));
	return msg;
}

std::unique_ptr<Message> MessageFactory::createOutboundMessage(MessageId id)
{
	std::unique_ptr<Message> msg(nullptr);
	if (id == MessageId::Call)
		msg.reset(new CallMessage());
	return msg;
}
