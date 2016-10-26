#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <string> 
#include "WKThermalChamber.hh"

WKThermalChamber cb;

typedef int com_rlfunc_t(const char *);
typedef struct {
  const char *name;
  com_rlfunc_t *func;
  const char *doc;
} COMMAND;


int com_help(const char *);
int com_quit(const char *);
int com_stat(const char *){};//TODO: print temp/hum/pos
int com_gotoinit(const char *){cb.Init();};
int com_gotox(const char *v){cb.MoveToPositionX(std::stoi(v));};
int com_gotoy(const char *v){cb.MoveToPositionY(std::stoi(v));};
int com_takephoto(const char *){cb.TakePhoto();};

void initialize_readline();
int execute_line(char *);
char *stripwhite(char *);
COMMAND *find_command(char *);
char *dupstr (const char *);


COMMAND commands[] = {
  { "help", com_help, "Display this text" },
  { "?", com_help, "Synonym for `help'" },
  { "quit", com_quit, "Quit" },
  { "stat", com_stat, "Print out state" },
  { "gotoinit", com_gotoinit, "Move the stage to postion ZERO" },
  { "gotox", com_gotox, "Move the stage to postion where X is N" },
  { "gotoy", com_gotoy, "Move the stage to postion where Y is N" },
  { "takephoto", com_takephoto, "Take a photo now" },
  { (char *)NULL, (com_rlfunc_t *)NULL, (char *)NULL }
};


int main (int argc, char **argv){
  char *line, *s;
  initialize_readline();
  cb.Init();
  while(1){
    line = readline ("RLThermal: ");
    if (!line)
      break;
    s = stripwhite (line);

    if (*s){
	add_history (s);
	execute_line (s);
    }
    free (line);
  }
  exit(0);
}


char *dupstr (const char *s){
  char *r;
  r = (char *)malloc (strlen (s) + 1);
  strcpy (r, s);
  return (r);
}

/* Execute a command line. */
int
execute_line (char *line)
{
  register int i;
  COMMAND *command;
  char *word;

  /* Isolate the command word. */
  i = 0;
  while (line[i] && whitespace (line[i]))
    i++;
  word = line + i;

  while (line[i] && !whitespace (line[i]))
    i++;

  if (line[i])
    line[i++] = '\0';

  command = find_command (word);

  if (!command)
    {
      fprintf (stderr, "%s: No such command for RLThermal.\n", word);
      return (-1);
    }

  /* Get argument to command, if any. */
  while (whitespace (line[i]))
    i++;

  word = line + i;

  /* Call the function. */
  return ((*(command->func)) (word));
}

/* Look up NAME as the name of a command, and return a pointer to that
   command.  Return a NULL pointer if NAME isn't a command name. */
COMMAND *
find_command (char *name)
{
  register int i;

  for (i = 0; commands[i].name; i++)
    if (strcmp (name, commands[i].name) == 0)
      return (&commands[i]);

  return ((COMMAND *)NULL);
}

/* Strip whitespace from the start and end of STRING.  Return a pointer
   into STRING. */
char *
stripwhite (char *string)
{
  register char *s, *t;

  for (s = string; whitespace (*s); s++)
    ;
    
  if (*s == 0)
    return (s);

  t = s + strlen (s) - 1;
  while (t > s && whitespace (*t))
    t--;
  *++t = '\0';

  return s;
}

/* **************************************************************** */
/*                                                                  */
/*                  Interface to Readline Completion                */
/*                                                                  */
/* **************************************************************** */

char *command_generator PARAMS((const char *, int));
char **fileman_completion PARAMS((const char *, int, int));

/* Tell the GNU Readline library how to complete.  We want to try to complete
   on command names if this is the first word in the line, or on filenames
   if not. */
void
initialize_readline ()
{
  /* Allow conditional parsing of the ~/.inputrc file. */
  rl_readline_name = "RLThermal";

  /* Tell the completer that we want a crack first. */
  rl_attempted_completion_function = fileman_completion;
}

/* Attempt to complete on the contents of TEXT.  START and END bound the
   region of rl_line_buffer that contains the word to complete.  TEXT is
   the word to complete.  We can use the entire contents of rl_line_buffer
   in case we want to do some simple parsing.  Return the array of matches,
   or NULL if there aren't any. */
char **
fileman_completion (const char*text, int start, int end)
{
  char **matches;

  matches = (char **)NULL;

  /* If this word is at the start of the line, then it is a command
     to complete.  Otherwise it is the name of a file in the current
     directory. */
  if (start == 0)
    matches = rl_completion_matches (text, command_generator);

  return (matches);
}

/* Generator function for command completion.  STATE lets us know whether
   to start from scratch; without any state (i.e. STATE == 0), then we
   start at the top of the list. */
char *
command_generator (const char*text, int state)
{
  static int list_index, len;
  const char *name;

  /* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
  if (!state)
    {
      list_index = 0;
      len = strlen (text);
    }

  /* Return the next name which partially matches from the command list. */
  while(name = commands[list_index].name)
    {
      list_index++;

      if (strncmp (name, text, len) == 0)
        return (dupstr(name));
    }

  /* If no names matched, then return NULL. */
  return ((char *)NULL);
}


/* Print out help for ARG, or for all of the commands if ARG is
   not present. */
int
com_help (const char *arg)
{
  register int i;
  int printed = 0;

  for (i = 0; commands[i].name; i++){
    if (!*arg || (strcmp (arg, commands[i].name) == 0)){
      printf ("%s\t\t%s.\n", commands[i].name, commands[i].doc);
      printed++;
    }
  }

  if (!printed){
    printf ("No commands match `%s'.  Possibilties are:\n", arg);
    
    for (i = 0; commands[i].name; i++){
      /* Print in six columns. */
      if (printed == 6)
	{
	  printed = 0;
	  printf ("\n");
	}
      printf ("%s\t", commands[i].name);
      printed++;
    }
    if (printed)
      printf ("\n");
  }
  return (0);
}

int com_quit (const char *arg){
  exit(0);
}
