/**
 * @file
 *
 * The .INI file routines
 */
#include "inifile.h"

#define INI_LINE_MAX 2048

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32)
#include <io.h>
#define strncasecmp _strnicmp
#define mkstemp(s) _mktemp_s(s, _countof(s))
#define close _close
#endif

#define nabto_assert(e)

static unsigned int returnDefaultValue(const char *Default,
                                       char *Buffer,
                                       unsigned int Size,
                                       int TwoNullChars);

unsigned int getIniString(const char *Section,
                          const char *KeyName,
                          const char *Default,
                          char *Buffer,
                          unsigned int Size,
                          const char *FileName)
{
   FILE          *InF;
   char           Buf[INI_LINE_MAX];
   unsigned int   BufLen;
   int            SectionFound;
   int            KeyNameFound;
   int            SectionLen;
   int            KeyLen;
   const char    *Ptr;
   unsigned int   Len;

   if (!FileName)
      return 0;
   if (!Default)
      return 0;
   if (!Buffer)
      return 0;
   if (!Size)
      return 0;

   /* Verify that strings are valid for the file format */
   if (Section && *Section && strchr(Section, ']'))
      return 0;
   if (KeyName && *KeyName && strchr(KeyName, '='))
      return 0;

   if (!Section)
   {
      /* Enumerate all sections in file */
      if (Size < 2)
         return 0;

      InF = fopen(FileName, "rt");
      if (!InF)
         return returnDefaultValue(Default, Buffer, Size, 1);

      Len = 0;
      while (fgets(Buf, sizeof(Buf), InF))
      {
         BufLen = strlen(Buf);
         if (BufLen > 0 && Buf[BufLen - 1] != '\n')
         {
            /* Line too long */
            fclose(InF);
            return returnDefaultValue(Default, Buffer, Size, 1);
         }

         if (Buf[0] == '[')
         {
            Ptr = strchr(Buf + 1, ']');
            if (Ptr)
            {
               BufLen = Ptr - (Buf + 1);
               if (Len + BufLen + 2 >= Size)
               {
                  nabto_assert(Size - Len - 2 < BufLen);
                  BufLen = Size - Len - 2;
                  strncpy(Buffer + Len, Buf + 1, BufLen);
                  Len += BufLen;
                  Buffer[Len++] = '\0';
                  Buffer[Len++] = '\0';
                  nabto_assert(Len == Size);
                  fclose(InF);
                  return Size - 2;
               }
               strncpy(Buffer + Len, Buf + 1, BufLen);
               Len += BufLen;
               Buffer[Len++] = '\0';
            }
         }
      }
      if (ferror(InF))
      {
         fclose(InF);
         return returnDefaultValue(Default, Buffer, Size, 1);
      }

      fclose(InF);
      Buffer[Len] = '\0';
      return Len;
   }

   if (!KeyName)
      if (Size < 2)
         return 0;

   InF = fopen(FileName, "rt");
   if (!InF)
      return returnDefaultValue(Default, Buffer, Size, KeyName == NULL);

   SectionFound = 0;

   /* Scan for [Section] in file */
   SectionLen = strlen(Section);
   while (fgets(Buf, sizeof(Buf), InF))
   {
      BufLen = strlen(Buf);
      if (BufLen > 0 && Buf[BufLen - 1] != '\n')
      {
         /* Line too long */
         fclose(InF);
         return returnDefaultValue(Default, Buffer, Size, KeyName == NULL);
      }

      if (Buf[0] == '[' &&
          strncasecmp(Buf + 1, Section, SectionLen) == 0 &&
          Buf[1 + SectionLen] == ']')
      {
         SectionFound = 1;
         break;
      }
   }
   if (ferror(InF))
   {
      fclose(InF);
      return returnDefaultValue(Default, Buffer, Size, KeyName == NULL);
   }

   if (!SectionFound)
   {
      fclose(InF);
      return returnDefaultValue(Default, Buffer, Size, KeyName == NULL);
   }

   if (!KeyName)
   {
      /* Enumerate all keys in section */
      Len = 0;
      while (fgets(Buf, sizeof(Buf), InF))
      {
         BufLen = strlen(Buf);
         if (BufLen > 0 && Buf[BufLen - 1] != '\n')
         {
            /* Line too long */
            fclose(InF);
            return returnDefaultValue(Default, Buffer, Size, 1);
         }

         if (Buf[0] == '[')
            break;

         Ptr = strchr(Buf, '=');
         if (Ptr)
         {
            BufLen = Ptr - Buf;
            if (Len + BufLen + 2 >= Size)
            {
               nabto_assert(Size - Len - 2 < BufLen);
               BufLen = Size - Len - 2;
               strncpy(Buffer + Len, Buf, BufLen);
               Len += BufLen;
               Buffer[Len++] = '\0';
               Buffer[Len++] = '\0';
               nabto_assert(Len == Size);
               fclose(InF);
               return Size - 2;
            }
            strncpy(Buffer + Len, Buf, BufLen);
            Len += BufLen;
            Buffer[Len++] = '\0';
         }
      }
      if (ferror(InF))
      {
         fclose(InF);
         return returnDefaultValue(Default, Buffer, Size, 1);
      }

      fclose(InF);
      Buffer[Len] = '\0';
      return Len;
   }

   /* Scan for KeyName= in section */
   KeyNameFound = 0;
   KeyLen = strlen(KeyName);
   while (fgets(Buf, sizeof(Buf), InF))
   {
      BufLen = strlen(Buf);
      if (BufLen > 0)
      {
         if (Buf[BufLen - 1] != '\n')
         {
            /* Line too long */
            fclose(InF);
            return returnDefaultValue(Default, Buffer, Size, 0);
         }
         BufLen--;
         if (BufLen > 0 && Buf[BufLen - 1] == '\r')
            BufLen--;
         Buf[BufLen] = '\0';
      }

      if (Buf[0] == '[')
         break;
      if (strncasecmp(Buf, KeyName, KeyLen) == 0 && Buf[KeyLen] == '=')
      {
         KeyNameFound = 1;
         break;
      }
   }
   if (ferror(InF))
   {
      fclose(InF);
      return returnDefaultValue(Default, Buffer, Size, 0);
   }
   fclose(InF);

   if (!KeyNameFound)
      return returnDefaultValue(Default, Buffer, Size, 0);

   Ptr = Buf + KeyLen + 1;
   switch (*Ptr)
   {
    case '"':
    case '\'':
      Len = strlen(Ptr);
      if (Len > 1 && Ptr[Len - 1] == Ptr[0])
      {
         Ptr++;
         Len -= 2;
      }
      break;
    default:
      Len = strlen(Ptr);
      break;
   }
   if (Len + 1 >= Size)
      Len = Size - 1;
   strncpy(Buffer, Ptr, Len);
   Buffer[Len] = '\0';

   return Len;
}

unsigned int returnDefaultValue(const char *Default,
                                char *Buffer,
                                unsigned int Size,
                                int TwoNullChars)
{
   unsigned int   Len;
   unsigned int   TermLen = TwoNullChars ? 2 : 1;

   if (!Buffer)
      return 0;
   if (!Size)
      return 0;
   nabto_assert(Size >= TermLen);

   Len = strlen(Default);
   if (Len + TermLen >= Size)
      Len = Size - TermLen;
   strncpy(Buffer, Default, Len);

   while (Len > 0 && strchr(" \t", Buffer[Len - 1]))
      Len--;

   Buffer[Len] = '\0';
   if (TwoNullChars)
      Buffer[Len + 1] = '\0';
   return Len;
}

int writeIniString(const char *Section,
                   const char *KeyName,
                   const char *String,
                   const char *FileName)
{
   FILE          *InF;
   char           TempFileName[256];
   int            fh;
   FILE          *OutF;
   char           Buf[INI_LINE_MAX];
   unsigned int   BufLen;
   int            SectionFound;
   int            NextSectionFound;
   int            KeyNameFound;
   int            SectionLen;
   int            KeyLen;

   if (!FileName)
      return 0;
   if (!Section)
      return 0;

   /* Verify that strings will not corrupt file format */
   if (Section && *Section && strchr(Section, ']'))
      return 0;
   if (KeyName && *KeyName && strchr(KeyName, '='))
      return 0;
   if (String && *String && strchr(String, '\n'))
      return 0;

   if (access(FileName, 0) != 0)
   {
      /* Create new file */
      if (!KeyName)
         return 1;
      if (!String)
         return 1;

      OutF = fopen(FileName, "wt");
      if (!OutF)
         return 0;

      /* Add section and key, value pair to file */
      if (fprintf(OutF, "[%s]\n%s=%s\n", Section, KeyName, String) < 0)
      {
         fclose(OutF);
         remove(FileName);
         return 0;
      }

      fclose(OutF);
      return 1;
   }

   /* Modify existing file */
   InF = fopen(FileName, "rt");
   if (!InF)
      return 0;

   sprintf(TempFileName, "%s.XXXXXX", FileName);
   fh = mkstemp(TempFileName);
   if (fh == -1)
   {
      fclose(InF);
      return 0;
   }

   OutF = fdopen(fh, "wt");
   if (!OutF)
   {
      fclose(InF);
      close(fh);
      remove(TempFileName);
      return 0;
   }

   SectionFound = 0;
   NextSectionFound = 0;

   /* Scan for [Section] in file */
   SectionLen = strlen(Section);
   while (fgets(Buf, sizeof(Buf), InF))
   {
      BufLen = strlen(Buf);
      if (BufLen > 0 && Buf[BufLen - 1] != '\n')
      {
         /* Line too long */
         fclose(InF);
         fclose(OutF);
         remove(TempFileName);
         return 0;
      }

      if (Buf[0] == '[' &&
          strncasecmp(Buf + 1, Section, SectionLen) == 0 &&
          Buf[1 + SectionLen] == ']')
      {
         SectionFound = 1;
         break;
      }

      if (fputs(Buf, OutF) == EOF)
      {
         fclose(InF);
         fclose(OutF);
         remove(TempFileName);
         return 0;
      }
   }
   if (ferror(InF))
   {
      fclose(InF);
      fclose(OutF);
      remove(TempFileName);
      return 0;
   }

   if (SectionFound)
   {
      if (KeyName)
      {
         /* Add this section (which we just read) */
         if (fputs(Buf, OutF) == EOF)
         {
            fclose(InF);
            fclose(OutF);
            remove(TempFileName);
            return 0;
         }

         /* Scan for KeyName= in section */
         KeyNameFound = 0;
         KeyLen = strlen(KeyName);
         while (fgets(Buf, sizeof(Buf), InF))
         {
            BufLen = strlen(Buf);
            if (BufLen > 0 && Buf[BufLen - 1] != '\n')
            {
               /* Line too long */
               fclose(InF);
               fclose(OutF);
               remove(TempFileName);
               return 0;
            }

            if (Buf[0] == '[')
            {
               NextSectionFound = 1;
               break;
            }
            if (strncasecmp(Buf, KeyName, KeyLen) == 0 && Buf[KeyLen] == '=')
            {
               KeyNameFound = 1;
               break;
            }

            if (fputs(Buf, OutF) == EOF)
            {
               fclose(InF);
               fclose(OutF);
               remove(TempFileName);
               return 0;
            }
         }
         if (ferror(InF))
         {
            fclose(InF);
            fclose(OutF);
            remove(TempFileName);
            return 0;
         }

         if (String)
         {
            if (KeyNameFound)
            {
               /* Change value for existing key */
               if (fprintf(OutF, "%.*s=%s\n", KeyLen, Buf, String) < 0)
               {
                  fclose(InF);
                  fclose(OutF);
                  remove(TempFileName);
                  return 0;
               }
            }
            else
            {
               /* Add new key value pair */
               if (fprintf(OutF, "%s=%s\n", KeyName, String) < 0)
               {
                  fclose(InF);
                  fclose(OutF);
                  remove(TempFileName);
                  return 0;
               }
            }
         }
      }
      else
      {
         /* Delete section by skipping it */
         while (fgets(Buf, sizeof(Buf), InF))
         {
            BufLen = strlen(Buf);
            if (BufLen > 0 && Buf[BufLen - 1] != '\n')
            {
               /* Line too long */
               fclose(InF);
               fclose(OutF);
               remove(TempFileName);
               return 0;
            }

            if (Buf[0] == '[')
            {
               NextSectionFound = 1;
               break;
            }
         }
         if (ferror(InF))
         {
            fclose(InF);
            fclose(OutF);
            remove(TempFileName);
            return 0;
         }
      }

      if (NextSectionFound)
      {
         /* Add next section (which we just read) */
         if (fputs(Buf, OutF) == EOF)
         {
            fclose(InF);
            fclose(OutF);
            remove(TempFileName);
            return 0;
         }
      }

      /* Copy rest of lines (if any) */
      while (fgets(Buf, sizeof(Buf), InF))
      {
         BufLen = strlen(Buf);
         if (BufLen > 0 && Buf[BufLen - 1] != '\n')
         {
            /* Line too long */
            fclose(InF);
            fclose(OutF);
            remove(TempFileName);
            return 0;
         }
         if (fputs(Buf, OutF) == EOF)
         {
            fclose(InF);
            fclose(OutF);
            remove(TempFileName);
            return 0;
         }
      }
      if (ferror(InF))
      {
         fclose(InF);
         fclose(OutF);
         remove(TempFileName);
         return 0;
      }
   }
   else
   {
      /* Section not found in file */
      if (KeyName)
      {
         if (String)
         {
            /* Add section and key, value pair to file */
            if (fprintf(OutF, "[%s]\n%s=%s\n", Section, KeyName, String) < 0)
            {
               fclose(InF);
               fclose(OutF);
               remove(TempFileName);
               return 0;
            }
         }
      }
   }

   fclose(InF);
   fclose(OutF);

   if (remove(FileName) != 0)
   {
       remove(TempFileName);
      return 0;
   }

   if (rename(TempFileName, FileName) != 0)
      return 0;

   return 1;
}

