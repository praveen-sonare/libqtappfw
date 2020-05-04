#ifndef UTILS_H
#define UTILS_H

#include <functional>
#include <QHash>
#include <QString>

namespace std {
	template<> struct hash<QString> {
		std::size_t operator()(const QString& s) const noexcept {
			return (size_t) qHash(s);
		}
	};
}

#endif
