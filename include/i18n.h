#ifndef BLESKOMAT_I18N_H
#define BLESKOMAT_I18N_H

#include "config.h"

#include <map>
#include <string>

namespace i18n {
	typedef std::map<const char*, const char*, util::MapCharPointerComparator> Locale;
	std::string t(const char* key);
	std::string t(const char* key, const char* locale);
	std::string getSupportedLocales();
}

#include "locale/cs.h"
#include "locale/da.h"
#include "locale/de.h"
#include "locale/en.h"
#include "locale/es.h"
#include "locale/fi.h"
#include "locale/fr.h"
#include "locale/hr.h"
#include "locale/it.h"
#include "locale/nl.h"
#include "locale/no.h"
#include "locale/pl.h"
#include "locale/pt.h"
#include "locale/ro.h"
#include "locale/sl.h"
#include "locale/sk.h"
#include "locale/sv.h"

#endif
