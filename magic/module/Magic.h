/* Magic.h
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef MAGIC_RUBY_EXTENSION_H
#define MAGIC_RUBY_EXTENSION_H

/* options */
#define MAGIC_OPT_CONTINUE "show_all"
#define MAGIC_OPT_SYMLINK "follow_symlinks"
#define MAGIC_OPT_RAW "allow_unprintable"
#define MAGIC_OPT_COMPRESS "archive_contents"
#define MAGIC_OPT_DEVICES "device_contents"
#define MAGIC_OPT_MIME "mime"
#define MAGIC_OPT_APPLE "apple"
#define MAGIC_OPT_MAGIC_FILE "magic_file"

/* module and method names */
#define MAGIC_MODULE_NAME "Magic"
#define MAGIC_METHOD_NAME "ext_identify"

void Init_MagicExt();

#endif
