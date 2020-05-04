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


#include <unordered_map>
#include <mutex>
#include <QDebug>
#include <QUrl>
#include "messageenginefactory.h"
#include "messageengine.h"
#include "utils.h"

std::shared_ptr<MessageEngine> MessageEngineFactory::getMessageEngine(const QUrl& url)
{
	static std::unordered_map<QString, std::shared_ptr<MessageEngine>> lut;
	static std::mutex m;

	std::lock_guard<std::mutex> localguard(m);
	auto  urlstr = url.toString();
	auto pme = lut[urlstr];
	if (!pme){
		pme  = std::make_shared<MessageEngine>(url);
		lut[urlstr] = pme;
	}

	return pme;
}
