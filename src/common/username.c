/*-------------------------------------------------------------------------
 *
 * username.c
 *	  get user name
 *
 * Portions Copyright (c) 1996-2014, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	  src/common/username.c
 *
 *-------------------------------------------------------------------------
 */

#ifndef FRONTEND
#include "postgres.h"
#else
#include "postgres_fe.h"
#endif

#include <errno.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>

#include "common/username.h"

/*
 * Returns the current user name in a static buffer, or NULL on error and
 * sets errstr
 */
const char *
get_user_name(char **errstr)
{
#ifndef WIN32
	struct passwd *pw;
	uid_t		user_id = geteuid();

	*errstr = NULL;

	errno = 0;					/* clear errno before call */
	pw = getpwuid(user_id);
	if (!pw)
	{
		*errstr = psprintf(_("failed to look up effective user id %ld: %s"),
						   (long) user_id,
						 errno ? strerror(errno) : _("user does not exist"));
		return NULL;
	}

	return pw->pw_name;
#else
	/* UNLEN = 256, 'static' variable remains after function exit */
	static char username[256 + 1];
	DWORD		len = sizeof(username) - 1;

	*errstr = NULL;

	if (!GetUserName(username, &len))
	{
		*errstr = psprintf(_("user name lookup failure: %s"), strerror(errno));
		return NULL;
	}

	return username;
#endif
}


/*
 * Returns the current user name in a static buffer or exits
 */
const char *
get_user_name_or_exit(const char *progname)
{
	const char *user_name;
	char	   *errstr;

	user_name = get_user_name(&errstr);

	if (!user_name)
	{
		fprintf(stderr, "%s: %s\n", progname, errstr);
		exit(1);
	}
	return user_name;
}
