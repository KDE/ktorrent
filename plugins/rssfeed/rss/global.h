/*
 * global.h
 *
 * Copyright (c) 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef LIBRSS_GLOBAL_H
#define LIBRSS_GLOBAL_H

template <class>
class QValueList;

namespace RSS
{
	/**
	 * Versions currently supported by this library. This enumeration is
	 * subject to be extended in the future and used by Document::version() to
	 * provide an interface to the client using which he can find out what
	 * version the loaded RSS file actually is.
	 */
	enum Version {
		v0_90,	/// RSS v0.90
		v0_91,	/// RSS v0.91
		v0_92,	/// RSS v0.92
		v0_93,	/// RSS v0.93
		v0_94,	/// RSS v0.94
		v1_0,	 /// RSS v1.0
		v2_0,	  /// RSS v2.0
		vAtom_0_1,  /// Atom v0.1
		vAtom_0_2,  /// Atom v0.2
		vAtom_0_3  /// Atom v0.3
	};
	
	/**
	 * Possible status values returned by the signal
	 * Loader::loadingComplete().
	 */
	enum Status {
		Success,	   /**
						* Nothing went wrong so far, but you still have to check
						* what values are returned by the classes since it's not
						* guaranteed that the retrieved RSS markup actually
						* complies to one of the RSS DTDs.*/
        Aborted,        /** the loader was aborted manually
                         */
		RetrieveError, /**
						* Something went wrong while retrieving the RSS data,
						* this could be a problem while resolving the host name
						* (assuming the source file loader was used) or a
						* problem with the program to be executed (in case the
						* program loader was used.).*/
		ParseError	 /**
						* The overall format of the RSS markup wasn't XML
						* conform. This only indicates that the data wasn't
						* valid (for example, if the data returned by a
						* DataRetriever isn't well-formed XML).
						* @see DataRetriever */
	};

	/**
	 * Possible languages which are returned by Document::language().
	 */
	enum Language {
		UndefinedLanguage, /** Unknown / undefined language */
		
		af,	/** Afrikaans */			  sq,	/** Albanian */
		eu,	/** Basque */				 be,	/** Belarusian */
		bg,	/** Bulgarian */			  ca,	/** Catalan */
		zh_cn, /** Chinese (Simplified) */   zh_tw, /** Chinese (Traditional */
		hr,	/** Croatian */			   cs,	/** Czech */
		da,	/** Danish */				 nl,	/** Dutch */
		nl_be, /** Dutch (Belgium) */		nl_nl, /** Dutch (Netherlands) */
		en,	/** English */				en_au, /** English (Australia) */
		en_bz, /** English (Belize) */	   en_ca, /** English (Canada) */
		en_ie, /** English (Ireland) */	  en_jm, /** English (Jamaica) */
		en_nz, /** English (New Zealand) */  en_ph, /** English (Phillipines) */
		en_za, /** English (South Africa) */ en_tt, /** English (Trinidad) */
		en_gb, /** English (Great Britain) */en_us, /** English (United States) */
		en_zw, /** English (Zimbabwe) */	 fo,	/** Faeroese */
		fi,	/** Finnish */				fr,	/** French */
		fr_be, /** French (Belgium) */	   fr_ca, /** French (Canada) */
		fr_fr, /** French (France) */		fr_lu, /** French (Luxembourg) */
		fr_mc, /** French (Monaco) */		fr_ch, /** French (Switzerland) */
		gl,	/** Galician */			   gd,	/** Gaelic */
		de,	/** German */				 de_at, /** German (Austria) */
		de_de, /** German (Germany) */	   de_li, /** German (Liechtenstein) */
		de_lu, /** German (Luxembourg) */	de_ch, /** German (Switzerland) */
		el,	/** Greek */				  hu,	/** Hungarian */
		is,	/** Icelandic */			  id,	/** Indonesian */
		ga,	/** Irish */				  it,	/** Italian */
		it_it, /** Italian (Italy) */		it_ch, /** Italian (Switzerland) */
		ja,	/** Japanese */			   ko,	/** Korean */
		mk,	/** Macedonian */			 no,	/** Norwegian */
		pl,	/** Polish */				 pt,	/** Portuguese */
		pt_br, /** Portuguese (Brazil) */	pt_pt, /** Portuguese (Portugal) */
		ro,	/** Romanian */			   ro_mo, /** Romanian (Moldova) */
		ro_ro, /** Romanian (Romania) */	 ru,	/** Russian */
		ru_mo, /** Russian (Moldova) */	  ru_ru, /** Russian (Russia) */
		sr,	/** Serbian */				sk,	/** Slovak */
		sl,	/** Slovenian */			  es,	/** Spanish */
		es_ar, /** Spanish (Argentina) */	es_bo, /** Spanish (Bolivia) */
		es_cl, /** Spanish (Chile) */		es_co, /** Spanish (Colombia) */
		es_cr, /** Spanish (Costa Rica) */   es_do, /** Spanish (Dominican Rep.) */
		es_ec, /** Spanish (Ecuador) */	  es_sv, /** Spanish (El Salvador) */
		es_gt, /** Spanish (Guatemala) */	es_hn, /** Spanish (Honduras) */
		es_mx, /** Spanish (Mexico) */	   es_ni, /** Spanish (Nicaragua) */
		es_pa, /** Spanish (Panama) */	   es_py, /** Spanish (Paraguay) */
		es_pe, /** Spanish (Peru) */		 es_pr, /** Spanish (Puerto Rico) */
		es_es, /** Spanish (Spain) */		es_uy, /** Spanish (Uruguay) */
		es_ve, /** Spanish (Venezuela) */	sv,	/** Swedish */
		sv_fi, /** Swedish (Finland) */	  sv_se, /** Swedish (Sweden) */
        tr,	/** Turkish */				uk	 /** Ukranian */
	};
	
	/**
	 * Possible values contained in a DayList.
	 */
	enum Day {
		UndefinedDay,
		Monday = 1, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday
	};

	enum Format {
		UnknownFormat,
		AtomFeed,
		RSSFeed
	};

	/**
	 * This type is used by Document::skipDays().
	 */
	typedef QValueList<Day> DayList;

	/**
	 * This type is used by Document::skipHours().
	 */
	typedef QValueList<unsigned short> HourList;
}

#endif // LIBRSS_GLOBAL_H
// vim: noet:ts=4
