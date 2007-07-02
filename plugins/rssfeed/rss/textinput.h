/*
 * textinput.h
 *
 * Copyright (c) 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef LIBRSS_TEXTINPUT_H
#define LIBRSS_TEXTINPUT_H

#include "global.h"

class KURL;

class QDomNode;
class QString;

namespace RSS
{
	/**
	 * Represents a text input facility as stored in a RSS file for the purpose
	 * of allowing users to submit queries back to the publisher's site. You
	 * don't have to instantiate one of these yourself, the common way to access
	 * instances is via Document::textInput().
	 * @see Document::textInput()
	 */
	class TextInput
	{
		public:
			/**
			 * Default constructor.
			 */
			TextInput();
			
			/**
			 * Copy constructor.
			 * @param other The TextInput object to copy.
			 */
			TextInput(const TextInput &other);
			
			/**
			 * Constructs a TextInput from a piece of RSS markup.
			 * @param node A QDomNode which references the DOM leaf to be used
			 * for constructing the TextInput.
			 */
			TextInput(const QDomNode &node);

			/**
			 * Assignment operator.
			 * @param other The TextInput object to clone.
			 * @return A reference to the cloned TextInput object.
			 */
			TextInput &operator=(const TextInput &other);

			/**
			 * Compares two text inputs. Two text inputs are considered
			 * identical if their properties (title, description, link etc.)
			 * are identical.
			 * @param other The text input to compare with.
			 * @return Whether the two text inputs are equal.
			 */
			bool operator==(const TextInput &other) const;
			
			/**
			 * Convenience method. Simply calls !operator==().
			 * @param other The text input to compared with.
			 * @return Whether the two text inputs are unequal.
			 */
			bool operator!=(const TextInput &other) const { return !operator==(other); }
			
			/**
			 * Destructor.
			 */
			virtual ~TextInput();

			/**
			 * RSS 0.90 and upwards
			 * @return The title (often a label to be used for the input field)
			 * of the text input, or QString::null if no title is available.
			 */
			QString title() const;

			/**
			 * RSS 0.90 and upwards
			 * @return The description (usually used as a tooltip which appears
			 * if the mouse hovers above the input field for a short time) of
			 * the text input, or QString::null if no description is
			 * available.
			 */
			QString description() const;

			/**
			 * RSS 0.90 and upwards
			 * @return The name of the text input (what's this for?) of the
			 * text input, or QString::null, if no name is available.
			 */
			QString name() const;

			/**
			 * RSS 0.90 and upwards
			 * @return A link to which the contents of the input field should
			 * be sent after the user specified them. This is often a CGI
			 * program on a remote server which evaluates the entered
			 * information. An empty KURL is returned in case no link is
			 * available.
			 * Note that the RSS 0.91 Specification dictates that URLs not
			 * starting with "http://" or "ftp://" are considered invalid.
			 */
			const KURL &link() const;

		private:
			struct Private;
			Private *d;
	};
}

#endif // LIBRSS_TEXTINPUT_H
// vim: noet:ts=4
