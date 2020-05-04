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

#ifndef MESSAGEENGINEFACTORY_H
#define MESSAGEENGINEFACTORY_H

#include <memory>

class MessageEngine;
class QUrl;


class MessageEngineFactory {
	public:
		static MessageEngineFactory& getInstance() {
			static MessageEngineFactory instance;
			return instance;
		}
		std::shared_ptr<MessageEngine> getMessageEngine(const QUrl &ur);
		MessageEngineFactory(MessageEngineFactory const&) = delete;
		void operator=(MessageEngineFactory const&) = delete;

	private:
		MessageEngineFactory() = default;
		~MessageEngineFactory() = default;
};

#endif // MESSAGENGINEEFACTORY_H
