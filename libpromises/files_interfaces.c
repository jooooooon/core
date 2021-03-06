/* 
   Copyright (C) Cfengine AS

   This file is part of Cfengine 3 - written and maintained by Cfengine AS.
 
   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; version 3.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License  
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA

  To the extent this program is licensed as part of the Enterprise
  versions of Cfengine, the applicable Commerical Open Source License
  (COSL) may apply to this file if you as a licensee so wish it. See
  included file COSL.txt.

*/

#include "files_interfaces.h"

#include "env_context.h"
#include "promises.h"
#include "dir.h"
#include "files_names.h"
#include "files_hashes.h"
#include "files_copy.h"
#include "item_lib.h"
#include "vars.h"
#include "matching.h"
#include "client_code.h"
#include "string_lib.h"
#include "rlist.h"

#ifdef HAVE_NOVA
#include "cf.nova.h"
#endif

int cfstat(const char *path, struct stat *buf)
{
#ifdef __MINGW32__
    return NovaWin_stat(path, buf);
#else
    return stat(path, buf);
#endif
}

/*********************************************************************/

int cf_lstat(char *file, struct stat *buf, FileCopy fc, AgentConnection *conn)
{
    if ((fc.servers == NULL) || (strcmp(fc.servers->item, "localhost") == 0))
    {
        return lstat(file, buf);
    }
    else
    {
        return cf_remote_stat(file, buf, "link", fc.encrypt, conn);
    }
}

ssize_t CfReadLine(char *buff, size_t size, FILE *fp)
{
    if (fgets(buff, size, fp) == NULL)
    {
        if (ferror(fp))
        {
            return -1;
        }
        else
        {
            return 0;
        }
    }

    /* We have got a line here */

    size_t line_length = strlen(buff);

    /* Check for \n */

    char *nl = strchr(buff, '\n');

    if (nl != NULL)
    {
        /* If we have found a \n, then line was read fully. */
        *nl = '\0';
        return line_length;
    }

    /* Read the remainder of the line */

    for (;;)
    {
        int c = fgetc(fp);
        if (c == EOF)
        {
            if (ferror(fp))
            {
                return -1;
            }
            else
            {
                /* We have reached EOF, report the length of line read so far */
                return line_length;
            }
        }

        line_length++;

        if (c == '\n')
        {
            return line_length;
        }
    }
}
