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

#ifndef MESSAGEFACTORY_H
#define MESSAGEFACTORY_H

#include <memory>
#include <QJsonDocument>

class Message;
enum class MessageId;

class MessageFactory {
	public:
		static MessageFactory& getInstance() {
			static MessageFactory instance;
			return instance;
		}
		std::unique_ptr<Message> createInboundMessage(MessageId id, QJsonDocument content);
		std::unique_ptr<Message> createOutboundMessage(MessageId);
		MessageFactory(MessageFactory const&) = delete;
		void operator=(MessageFactory const&) = delete;

	private:
		MessageFactory() = default;
		~MessageFactory() = default;
};

#endif // MESSAGEFACTORY_H