/*
 * Copyright (C) 2019 Konsulko Group
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

#include "voiceagentprofile.h"

VoiceAgentProfile::VoiceAgentProfile(const QString &name,
				     const QString &id,
				     const QString &api,
				     bool active,
				     const QString &wuw,
				     const QString &vendor,
				     const QString &wuws)
	: m_name(name), m_vaid(id), m_vaapi(api), m_active(active),
	  m_activewuw(wuw), m_vendor(vendor), m_wuws(wuws),
	  m_authstate("UNINITIALIZED"), m_connstate("DISCONNECTED"),
	  m_dialogstate("MICROPHONEOFF"), m_logincode(QString()),
	  m_loginurl(QString()), m_expired(true)
{
}

QString VoiceAgentProfile::name() const
{
	return m_name;
}

QString VoiceAgentProfile::vaid() const
{
	return m_vaid;
}

QString VoiceAgentProfile::vaapi() const
{
	return m_vaapi;
}

bool VoiceAgentProfile::isactive() const
{
	return m_active;
}

QString VoiceAgentProfile::activewuw() const
{
	return m_activewuw;
}

QString VoiceAgentProfile::vendor() const
{
	return m_vendor;
}

QString VoiceAgentProfile::wuws() const
{
	return m_wuws;
}

QString VoiceAgentProfile::authstate() const
{
	return m_authstate;
}

QString VoiceAgentProfile::connstate() const
{
	return m_connstate;
}

QString VoiceAgentProfile::dialogstate() const
{
	return m_dialogstate;
}

QString VoiceAgentProfile::logincode() const
{
	return m_logincode;
}

QString VoiceAgentProfile::loginurl() const
{
	return m_loginurl;
}

bool VoiceAgentProfile::isloginpairexpired() const
{
	return m_expired;
}

void VoiceAgentProfile::setVaid(const QString id)
{
	m_vaid = id;
}

void VoiceAgentProfile::setActive(bool active)
{
	m_active = active;
}

void VoiceAgentProfile::setAuthState(const QString state)
{
	m_authstate = state;
}

void VoiceAgentProfile::setConnState(const QString state)
{
	m_connstate = state;
}

void VoiceAgentProfile::setDialogState(const QString state)
{
	m_dialogstate = state;
}

void VoiceAgentProfile::setLoginCode(const QString usrcode)
{
	m_logincode = usrcode;
}

void VoiceAgentProfile::setLoginUrl(const QString usrurl)
{
	m_loginurl = usrurl;
}

void VoiceAgentProfile::setLoginPairExpired(bool expired)
{
	m_expired = expired;
}

void VoiceAgentProfile::setWuw(const QString newwuw)
{
	m_activewuw = newwuw;
}
