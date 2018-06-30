#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <unistd.h>

#define ERR_INVALID_INPUT_IP      -2
#define ERR_INVALID_REGEX_PATTERN -3
#define ERR_INVALID_OPTION        -4

typedef unsigned char ui8;

typedef enum {

  CIDR_PRINT_NORMAL,
  CIDR_PRINT_SCRIPT

} cidrPrintStyles;

typedef struct {

  ui8 A;
  ui8 B;
  ui8 C;
  ui8 D;

} ipField;

typedef struct {

  ui8 A;
  ui8 B;
  ui8 C;
  ui8 D;

} ipMask;

void printUsage(const char *programName);

const char *cidrIpToString(const ipField *field, const ipMask *mask);
const char *cidrIpMaskToString(const ipMask *mask);

ui8 netSlashNotation(const ipField *field1, const ipField *field2, const ipMask *netMask)
{
  ui8 i, j;
  ui8 result = 0;

  for(j = 0; j < 4; j++)
    {
      ui8 n     = 0;
      ui8 *a    = (ui8 *) &field1->A + j;
      ui8 *b    = (ui8 *) &field2->A + j;
      ui8 *mask = (ui8 *) netMask + j;

      ui8 c = *a & 0x80;
      ui8 d = *b & 0x80;
      for(i = 0; c == d && i < 8; i++)
	{
	  c = *a & (0x80 >> i);
	  d = *b & (0x80 >> i);
	  *mask = (((c == d) << (7 - i)) | ((unsigned int) *mask)) & 0xFFFF;
	  n += (c == d);
	}
      result += n;
    }

  return result;
}

const char *cidrIpToString(const ipField *field, const ipMask *mask)
{
  static char str[20];
  sprintf(str, "%u.%u.%u.%u",
	  field->A & mask->A,
	  field->B & mask->B,
	  field->C & mask->C,
	  field->D & mask->D);
  return str;
}

const char *cidrIpMaskToString(const ipMask *mask)
{
  static char str[20];
  sprintf(str,
	  "%hhu.%hhu.%hhu.%hhu",
	  mask->A,
	  mask->B,
	  mask->C,
	  mask->D);
  return str;
}

void printUsage(const char *programName)
{
  printf("\nUsage: %s options ip1 ip2\n" \
	 "\n"
	 "Options\n"
	 "  -b : output for scripts\n"
	 "\n",
	 programName);
  exit(ERR_INVALID_OPTION);
}

int main(int argc, char **argv)
{
  if(argc < 2)
    printUsage(argv[0]);

  int ipRegExResult;
  regex_t start_state;
  const char *ipRegExPattern = "^[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}$";

  if(regcomp(&start_state, ipRegExPattern, REG_EXTENDED))
    {
      fprintf(stderr, "Invalid RegEx pattern, exiting....\n");
      exit(ERR_INVALID_REGEX_PATTERN);
    }
  if((ipRegExResult = regexec(&start_state, argv[argc - 2], 0, NULL, 0)) != 0)
    {
      fprintf(stderr, "ip1 has invalid format, exiting....\n");
      printUsage(argv[0]);
      exit(ERR_INVALID_INPUT_IP);
    }
  if((ipRegExResult = regexec(&start_state, argv[argc - 1], 0, NULL, 0)) != 0)
    {
      fprintf(stderr, "ip2 has invalid format, exiting....\n");
      printUsage(argv[0]);
      exit(ERR_INVALID_INPUT_IP);
    }

  int c;
  cidrPrintStyles printStyle = CIDR_PRINT_NORMAL;
  opterr = 0;

  while ((c = getopt (argc, argv, "abc:")) != -1)
    switch (c)
      {
      case 'b':
	printStyle = CIDR_PRINT_SCRIPT;
	break;
      case '?':
	if (isprint(optopt))
	  {
	    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
	    printUsage(argv[0]);
	    exit(ERR_INVALID_OPTION);
	  }
	else
	  {
	    fprintf(stderr, "Unknown option character `\\x%x'.\n",optopt);
	    printUsage(argv[0]);
	    exit(ERR_INVALID_OPTION);
	  }
      default:
	abort ();
      }

  ipField field1, field2;
  ipMask netMask;
  memset(&netMask, 0, sizeof(netMask));

  int i;
  char *fieldSaveTok1, *fieldSaveTok2;
  char *fieldTok1 = strtok_r(argv[argc - 2], ".", &fieldSaveTok1);
  char *fieldTok2 = strtok_r(argv[argc - 1], ".", &fieldSaveTok2);
  for(i = 0; i < 4; i++)
    {
      ui8* fieldPtr1 = (ui8*) &field1.A + i;
      ui8* fieldPtr2 = (ui8*) &field2.A + i;

      *fieldPtr1 = (ui8) atoi(fieldTok1);
      *fieldPtr2 = (ui8) atoi(fieldTok2);

      fieldTok1 = strtok_r(NULL, ".", &fieldSaveTok1);
      fieldTok2 = strtok_r(NULL, ".", &fieldSaveTok2);
    }

  int result = netSlashNotation(&field1, &field2, &netMask);

  unsigned int nrIps = 0;
  nrIps |= (~netMask.A & 0xFF) << 24;
  nrIps |= (~netMask.B & 0xFF) << 16;
  nrIps |= (~netMask.C & 0xFF) << 8;
  nrIps |= (~netMask.D & 0xFF);
  nrIps += 1;

  switch(printStyle)
    {
    case CIDR_PRINT_NORMAL:

      printf("\n%s/%u, (%d)\n",
	     cidrIpToString(&field1, &netMask),
	     result,
	     nrIps);
      printf("netmask: %s\n\n", cidrIpMaskToString(&netMask));
      break;
    case CIDR_PRINT_SCRIPT:

      printf("%s|%u|%d|%s\n",
	     cidrIpToString(&field1, &netMask),
	     result,
	     nrIps,
	     cidrIpMaskToString(&netMask)
	     );
      break;
    }

  return 0;
}
