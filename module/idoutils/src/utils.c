/***************************************************************
 * UTILS.C - IDO Utils
 *
 * Copyright (c) 2005-2008 Ethan Galstad
 * Copyright (c) 2009-2013 Icinga Development Team (http://www.icinga.org)
 *
 **************************************************************/

#include "../../../include/config.h"
#include "../include/common.h"
#include "../include/utils.h"




/****************************************************************************/
/* DYNAMIC BUFFER FUNCTIONS                                                 */
/****************************************************************************/

/* initializes a dynamic buffer */
int ido_dbuf_init(ido_dbuf *db, int chunk_size) {

	if (db == NULL)
		return IDO_ERROR;

	db->buf = NULL;
	db->used_size = 0L;
	db->allocated_size = 0L;
	db->chunk_size = chunk_size;

	return IDO_OK;
}


/* frees a dynamic buffer */
int ido_dbuf_free(ido_dbuf *db) {

	if (db == NULL)
		return IDO_ERROR;

	if (db->buf != NULL)
		free(db->buf);
	db->buf = NULL;
	db->used_size = 0L;
	db->allocated_size = 0L;

	return IDO_OK;
}


/* dynamically expands a string */
int ido_dbuf_strcat(ido_dbuf *db, char *buf) {
	char *newbuf = NULL;
	unsigned long buflen = 0L;
	unsigned long new_size = 0L;
	unsigned long memory_needed = 0L;

	if (db == NULL || buf == NULL)
		return IDO_ERROR;

	/* how much memory should we allocate (if any)? */
	buflen = strlen(buf);
	new_size = db->used_size + buflen + 1;

	/* we need more memory */
	if (db->allocated_size < new_size) {

		memory_needed = ((ceil(new_size / db->chunk_size) + 1) * db->chunk_size);

		/* allocate memory to store old and new string */
		if ((newbuf = (char *)realloc((void *)db->buf, (size_t)memory_needed)) == NULL)
			return IDO_ERROR;

		/* update buffer pointer */
		db->buf = newbuf;

		/* update allocated size */
		db->allocated_size = memory_needed;

		/* terminate buffer */
		db->buf[db->used_size] = '\x0';
	}

	/* append the new string */
	strcat(db->buf, buf);

	/* update size allocated */
	db->used_size += buflen;

	return IDO_OK;
}



/******************************************************************/
/************************* FILE FUNCTIONS *************************/
/******************************************************************/

/* renames a file - works across filesystems (Mike Wiacek) */
int my_rename(char *source, char *dest) {
	char buffer[1024] = {0};
	int rename_result = 0;
	int source_fd = -1;
	int dest_fd = -1;
	int bytes_read = 0;


	/* make sure we have something */
	if (source == NULL || dest == NULL)
		return -1;

	/* first see if we can rename file with standard function */
	rename_result = rename(source, dest);

	/* handle any errors... */
	if (rename_result == -1) {

		/* an error occurred because the source and dest files are on different filesystems */
		if (errno == EXDEV) {

			/* open destination file for writing */
			if ((dest_fd = open(dest, O_WRONLY | O_TRUNC | O_CREAT | O_APPEND, 0644)) > 0) {

				/* open source file for reading */
				if ((source_fd = open(source, O_RDONLY, 0644)) > 0) {

					while ((bytes_read = read(source_fd, buffer, sizeof(buffer))) > 0)
						write(dest_fd, buffer, bytes_read);

					close(source_fd);
					close(dest_fd);

					/* delete the original file */
					unlink(source);

					/* reset result since we successfully copied file */
					rename_result = 0;
				}

				else {
					close(dest_fd);
					return rename_result;
				}
			}
		}

		else {
			return rename_result;
		}
	}

	return rename_result;
}




/******************************************************************/
/************************ STRING FUNCTIONS ************************/
/******************************************************************/

/* strip newline, carriage return, and tab characters from beginning and end of a string */
void idomod_strip(char *buffer) {
	register int x = 0;
	register int y = 0;
	register int z = 0;

	if (buffer == NULL || buffer[0] == '\x0')
		return;

	/* strip end of string */
	y = (int)strlen(buffer);
	for (x = y - 1; x >= 0; x--) {
		if (buffer[x] == ' ' || buffer[x] == '\n' || buffer[x] == '\r' || buffer[x] == '\t' || buffer[x] == 13)
			buffer[x] = '\x0';
		else
			break;
	}
	/* save last position for later... */
	z = x;

	/* strip beginning of string (by shifting) */
	for (x = 0;; x++) {
		if (buffer[x] == ' ' || buffer[x] == '\n' || buffer[x] == '\r' || buffer[x] == '\t' || buffer[x] == 13)
			continue;
		else
			break;
	}
	if (x > 0) {
		/* new length of the string after we stripped the end */
		y = z + 1;

		/* shift chars towards beginning of string to remove leading whitespace */
		for (z = x; z < y; z++)
			buffer[z-x] = buffer[z];
		buffer[y-x] = '\x0';
	}

	return;
}
