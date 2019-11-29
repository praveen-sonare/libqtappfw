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

#ifndef VOICEAGENTPROFILE_H
#define VOICEAGENTPROFILE_H

#include <QString>

class VoiceAgentProfile
{
	public:
		VoiceAgentProfile(const QString &name,
				  const QString &id,
				  const QString &api,
				  bool active,
				  const QString &wuw,
				  const QString &vendor,
				  const QString &wuws);

		QString name() const;
		QString vaid() const;
		QString vaapi() const;
		bool isactive() const;
		QString activewuw() const;
		QString vendor() const;
		QString wuws() const;
		QString authstate() const;
		QString connstate() const;
		QString dialogstate() const;
		QString logincode() const;
		QString loginurl() const;
		bool isloginpairexpired() const;

		void setVaid(const QString newid);
		void setActive(bool activemode);
		void setAuthState(const QString newauthstate);
		void setConnState(const QString newconnstate);
		void setDialogState(const QString newdialogstate);
		void setLoginCode(const QString newtoken);
		void setLoginUrl(const QString newurl);
		void setLoginPairExpired(bool expired);
		void setWuw(const QString newwuw);

		bool operator==(const VoiceAgentProfile& rhs) {
			return (m_name == rhs.name() &&
				m_vaid == rhs.vaid() &&
				m_vaapi == rhs.vaapi()); };

	private:
		QString m_name;
		QString m_vaid;
		QString m_vaapi;
		bool m_active;
		QString m_activewuw;
		QString m_vendor;
		QString m_wuws;
		QString m_authstate;
		QString m_connstate;
		QString m_dialogstate;
		QString m_logincode;
		QString m_loginurl;
		bool m_expired;
};

#endif // VOICEAGENTPROFILE_H
