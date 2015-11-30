#ifndef INIFILE_H
#define INIFILE_H
/**
 * @file
 *
 * The prototypes for the .INI file routines
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  Retrieves a string from the specified section in an
 *  initialization file.
 *  This function searches the specified initialization file for a key that
 *  matches the name specified by the KeyName parameter under the section
 *  heading specified by the Section parameter. If it finds the key, the
 *  function copies the corresponding string to the buffer. If the key does
 *  not exist, the function copies the default character string specified
 *  by the Default parameter. A section in the initialization file must have
 *  the following form:
 *
 *    [section]
 *    key=string
 *    .
 *    .
 *    .
 *
 *  If Section is NULL, rhis function copies all section names in the
 *  specified file to the supplied buffer. If KeyName is NULL, the
 *  function copies all key names in the specified section to the supplied
 *  buffer. An application can use this method to enumerate all of the
 *  sections and keys in a file. In either case, each string is followed
 *  by a null character and the final string is followed by a second null
 *  character. If the supplied destination buffer is too small to hold all
 *  the strings, the last string is truncated and followed by two null
 *  characters.
 *
 *  If the string associated with KeyName is enclosed in single or double
 *  quotation marks, the marks are discarded when this function retrieves
 *  the string.
 *
 * @param Section - Pointer to a null-terminated string that specifies
 *  the name of the section containing the key name. If this parameter is
 *  NULL, this function copies all section names in the file to the supplied
 *  buffer. The name of the section is case-independent;  the string can be
 *  any combination of characters except for the right square bracket (]).
 *
 * @param KeyName - Pointer to the null-terminated string specifying the name of
 *  the key whose associated string is to be retrieved. If this parameter
 *  is NULL, all key names in the section specified by the Section parameter
 *  are copied to the buffer specified by the Buffer parameter. The name
 *  of the key is case-independent; the string can be any combination of
 *  characters except for the equal sign (=).
 *
 * @param Default - Pointer to a null-terminated default string. If the KeyName
 *  key cannot be found in the initialization file, this function copies
 *  the default string to the buffer. This parameter cannot be NULL.
 *  Avoid specifying a default string with trailing blank characters. The
 *  function inserts a null character in the buffer to strip any trailing
 *  blanks.
 *
 * @param Buffer - Pointer to the buffer that receives the retrieved string.
 *
 * @param Size - Size of the buffer pointed to by the Buffer parameter, in
 *  bytes.
 *
 * @param FileName - Pointer to a null-terminated string that specifies the name
 *  of the initialization file. If this parameter does not contain a full
 *  path to the file, the system searches for the file in the current
 *  directory.
 *
 * @return The return value is the number of characters copied to the
 *  buffer, not including the terminating null character.
 *
 *  If neither Section nor KeyName is NULL and the supplied destination
 *  buffer is too small to hold the requested string, the string is
 *  truncated and followed by a null character, and the return value is
 *  equal to Size minus one.
 *
 *  If either Section or KeyName is NULL and the supplied destination
 *  buffer is too small to hold all the strings, the last string is
 *  truncated and followed by two null characters. In this case, the
 *  return value is equal to Size minus two.
**/
extern unsigned int getIniString(const char *Section,
                                 const char *KeyName,
                                 const char *Default,
                                 char *Buffer,
                                 unsigned int Size,
                                 const char *FileName);


/**
 *  Copies a string into the specified section of an
 *  initialization file.
 *  A section in the initialization file must have the following form:
 *
 *    [section]
 *    key=string
 *    .
 *    .
 *    .
 *
 *  If FileName contains a full path and file name and the file does not
 *  exist, the file will be created. The specified directory must already
 *  exist.
 *
 *  If the FileName parameter does not contain a full path and file name
 *  for the file, the current directory is sought for the file. If the file
 *  does not exist, this function creates the file in the current directory.
 *
 * @param Section - Pointer to a null-terminated string containing the
 *  name of the section to which the string will be copied. If the section
 *  does not exist, it is created. The name of the section is
 *  case-independent;  the string can be any combination of characters
 *  except for the right square bracket (]).
 *
 * @param KeyName - Pointer to the null-terminated string containing the name of
 *  the key to be associated with a string. If the key does not exist in the
 *  specified section, it is created. If this parameter is NULL, the entire
 *  section, including all entries within the section, is deleted. The
 *  name of the key is case-independent; the string can be any
 *  combination of characters except for the equal sign (=).
 *
 * @param String - Pointer to a null-terminated string to be written to the file.
 *  If this parameter is NULL, the key pointed to by the lpKeyName parameter
 *  is deleted.
 *
 * @param FileName - Pointer to a null-terminated string that specifies the name
 *  of the initialization file.
 *
 * @return  1 if ok, 0 otherwise
**/
extern int writeIniString(const char *Section,
                          const char *KeyName,
                          const char *String,
                          const char *FileName);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* INIFILE_H */
