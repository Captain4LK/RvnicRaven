#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"

#define HLH_IMPLEMENTATION
#include "HLH.h"

typedef struct
{
   int verbose;
   char *project_file;
   char *platform;
   char *target;
}RvR_make_options;

typedef struct
{
   char *target;
   char *target_type;
   char *outdir;

   char **src;
   char **inc;
}RvR_make_target;

typedef struct
{
   int line_on;
   char *file_on;
   char *file_contents;

   char abs_path[1024];

   RvR_make_target target;
}RvR_make_state;

typedef enum
{
   ALREADY_UP_TO_DATE = 0,
   CHANGED = 1,
   BUILD_ERROR = 2,
   NO_MORE_TARGETS = 3,
}RvR_make_target_state;

//TODO: (re)move this to somewhere else
enum
{ BUF_SIZE = 1024 * 1000 };
char *buf = 0;
int boff = 0;

#define MAX_LEN 1000

RvR_make_options make_options = {0};
RvR_make_state make_state = {0};

static int make_get_target(RvR_make_state *state, RvR_make_target *target, RvR_make_target *top_target, char **p, char *target_name_to_match);

static char *get_token(RvR_make_state *state, char **p);
static char skip_white(RvR_make_state *state, char **p);
static char *buf_alloc(char *p, int len);
static void next_line(RvR_make_state *state, char **p);
static char *form(char *format, ...);

static char *load_file(RvR_make_state *state, char *fn, int set_current_file);
static void make_error(RvR_make_state *state, char *format, ...);
static void print_help(char **argv);

int main(int argc, char **argv)
{
   struct optparse_long longopts[] =
   {
      {"verbose", 'v', OPTPARSE_NONE},
      {"file", 'f', OPTPARSE_REQUIRED},
      {"platform", 'p', OPTPARSE_REQUIRED},
      {"target", 't', OPTPARSE_REQUIRED},
      {"help", 'h', OPTPARSE_NONE},
      {0},
   };

   int option;
   struct optparse options;
   optparse_init(&options, argv);
   while((option = optparse_long(&options, longopts, NULL))!=-1)
   {
      switch(option)
      {
      case 'v':
         make_options.verbose = 1;
         break;
      case 'f':
         make_options.project_file = options.optarg;
         break;
      case 'p':
         make_options.platform = options.optarg;
         break;
      case 't':
         make_options.target = options.optarg;
         break;
      case 'h':
         print_help(argv);
         exit(EXIT_SUCCESS);
      case '?':
         fprintf(stderr, "%s: %s\n", argv[0], options.errmsg);
         exit(EXIT_FAILURE);
         break;
      }
   }

   if(make_options.project_file==NULL)
      return 1;

   strcpy(make_state.abs_path,"./");

   char *file_start = load_file(&make_state,make_options.project_file,1);
   if(!file_start)
      make_error(&make_state,"Project file not found: '%s'\n", make_options.project_file);

   make_get_target(

   return 0;
}

static int make_get_target(RvR_make_state *state, RvR_make_target *target, RvR_make_target *top_target, char **p, char *target_name_to_match)
{
   int ret = ALREADY_UP_TO_DATE;

   for(;;)
   {
      char *left_brace = get_token(state,p);
      if(left_brace)
      {
         if(strcmp(left_brace, "["))
            make_error(state,"expecting '['");

         mk_reset(target);
         target->target_type = get_token(state,p);
         target->target = get_token(state,p);

         if(strcmp(get_token(state,p), "]"))
            make_error(state,"expecting ']'");

         // should we use this target or skip it
         if(target_name_to_match
            && strcmp(target->target, target_name_to_match))
         {
            mk_debug("skipping section %s\n", target->target);
            skip_white(state,p);
            while(**p && **p != '[')
            {
               get_token(state,p);
               skip_white(state,p);
            }
         }
         else
         {
            mk_debug("processing section %s\n", target->target);
            skip_white(state,p);
            while(**p && **p != '[')
            {
               char *t = get_token(state,p);

               if(strcmp(t, "win32") == 0)
               {
                  /*if(mk_options.os != OS_WIN32)
                  {
                     mk_debug("%s : skipping line, wrong os\n", t);
                     next_line(p);
                  }*/
                  next_line(state,p);
               }
               else if(strcmp(t, "linux") == 0)
               {
                  /*if(mk_options.os != OS_LINUX)
                  {
                     mk_debug("%s : skipping line, wrong os\n", t);
                     next_line(p);
                  }*/
                  next_line(state,p);
               }
               else if(strcmp(t, "opt") == 0)
               {
                  /*if(current_build_type != BUILD_OPT)
                  {
                     mk_debug("%s : skipping line, wrong build type\n", t);
                     next_line(p);
                  }*/
                  next_line(state,p);
               }
               else if(strcmp(t, "debug") == 0)
               {
                  /*if(current_build_type != BUILD_DEBUG)
                  {
                     mk_debug("%s : skipping line, wrong build type\n", t);
                     next_line(p);
                  }*/
                  next_line(state,p);
               }
               else if(strcmp(t, "profile") == 0)
               {
                  /*if(current_build_type != BUILD_PROF)
                  {
                     mk_debug("%s : skipping line, wrong build type\n", t);
                     next_line(p);
                  }*/
                  next_line(state,p);
               }
               else if(strcmp(t, "hostname") == 0)
               {
                  t = get_token(state,p);
                  char name[200];
                  gethostname(name, 200);
                  if(strcmp(t, name) != 0)
                     next_line(state,p);
               }
               else if(strcmp(t, "nothostname") == 0)
               {
                  t = get_token(state,p);
                  char name[200];
                  gethostname(name, 200);
                  if(strcmp(t, name) == 0)
                     next_line(state,p);
               }
               //else if(strcmp(t, "link_as_windowed_app") == 0)
                  //top_target->app_type = WIN32_WINDOWED_APP;
               else if(strcmp(t, "target_dir") == 0)
               {
                  char *dir = get_token(state,p);
                  MKDIR(dir);
                  target->outdir = form("%s/", dir);
               }
               else if(strcmp(t, "add_to_executable") == 0)
               {
                  char *fn = get_token(state,p);
                  /*if(fn[0] == '-' && fn[1] == 'l')
                  {
                     if(table_find(top_target->libs, fn) == -1)
                        table_add(top_target->libs, strdup(fn), -1);
                  }
                  else
                  {

                     char *file = get_abs_path(fn, 0);
                     char *d = file + strlen(file);
                     while(d > file && *d != '.')
                        d--;

                     if(strcmp(top_target->target_type, "executable") == 0)
                     {
                        if(strcmp(d, ".def") == 0)
                           top_target->def_file = file;
                        else if(strcmp(d, ".a") == 0 ||
                                (strstr(file, ".so") != 0) ||
                                strcmp(d, ".lib") == 0
                                || strcmp(d, ".res") == 0)
                        {
                           if(strcmp(d, ".res") == 0)
                              add_to_backup(file);

                           if(table_find(top_target->libs, file) == -1)
                              table_add(top_target->libs, file, -1);
                        }
                        else if(table_find(top_target->src, file) == -1)
                           table_add(top_target->src, file, -1);
                     }
                  }*/
               }
               else if(strcmp(t, "backup") == 0)
               {
                  char *fname = get_token(state,p);
                  add_to_backup(fname);
               }
               else if(strcmp(t, "add_include_directory") == 0)
               {
                  char *i = get_abs_path(get_token(state,p), 1);

                  if(table_find(target->inc, i) == -1)
                     table_add(target->inc, i, -1);

                  if(table_find(top_target->inc, i) == -1)
                     table_add(top_target->inc, i, -1);

                  mk_debug("add_include_directory (dir=%s)\n", i);
               }
               else if(strcmp(t, "add_define") == 0)
               {
                  char *i = get_token(state,p);

                  //if(table_find(target->defines, i) == -1)
                     //table_add(target->defines, i, -1);

                  make_debug(state,"add_define (define=%s)\n", i);
               }
               else if(strcmp(t, "add_global_define") == 0)
               {
                  char *i = get_token(state,p);

                  //if(table_find(&mk_global_defines, i) == -1)
                     //table_add(&mk_global_defines, i, -1);

                  make_debug(state,"add_global_define (define=%s)\n", i);
               }
               else if(strcmp(t, "ram_file") == 0)
               {
                  char *in_name = get_token(state,p);
                  int l = state->line_on;
                  skip_white(state,p);

                  /*if(current_build_type == BUILD_BACKUP)
                  {
                     add_to_backup(in_name);
                     if(line_on == l)
                        get_token(state,p);
                  }
                  else
                  {
                     if(line_on == l)
                        build_ram_file(in_name, get_token(state,p), top_target);
                     else
                        build_ram_file(in_name, in_name, top_target);
                  }*/


               }
               else if(strcmp(t, "use") == 0)
               {

                  t = get_token(state,p);
                  char *use_file = 0;


                  if(strcmp(t, "file") == 0)
                  {
                     if(strcmp(get_token(state,p), "="))
                        make_error(state,"expected = after file");
                     use_file = get_token(state,p);
                     t = get_token(state,p);

                     add_to_backup(use_file);
                  }

                  mk_debug("use (file=%s) (target=%s)\n", use_file, t);


                  RvR_make_target tar = {0};
                  //tar.dlls = table_create(20, 100);
                  //tar.libs = table_create(20, 100);
                  //tar.src = table_create(20, 100);
                  //tar.inc = table_create(20, 100);
                  //tar.defines = table_create(20, 100);

                  int cur_line = state->line_on;       // save current line & file info
                  char *cur_file = state->file_on;
                  char *cur_contents = state->file_contents;

                  char old_abs[MAX_LEN];
                  char *f;
                  if(use_file)
                  {
                     f = load_file(state,use_file, 1);
                     if(!f)
                        make_error(state,"could not load project file '%s'",
                                 use_file);

                     strcpy(old_abs, state->abs_path);
                     set_abs_path(use_file);
                     free(f);
                  }
                  else
                     f = state->file_contents;


                  if(get_target(&tar, top_target, &f, t) == NO_MORE_TARGETS)
                     make_error(state,"could not find target '%s'", t);

                  // add include paths to local target
                  for(int i = 0; i < table_size(tar.inc); i++)
                     if(table_find(target->inc, table_element(tar.inc, i)) ==
                        -1)
                        table_add(target->inc, table_element(tar.inc, i), -1);

                  ret = build_target(&tar, top_target);

                  if(use_file)
                  {
                     state->file_contents = cur_contents;
                     state->file_on = cur_file;
                     set_abs_path(old_abs);
                  }

                  state->line_on = cur_line;

                  //TODO
                  //table_destroy(tar.dlls);
                  //table_destroy(tar.libs);
                  //table_destroy(tar.src);
                  //table_destroy(tar.inc);
                  //table_destroy(tar.defines);
               }
               else
               {
                  mk_debug("add to target %s\n", t);
                  table_add(target->src, t, -1);
               }

               skip_white(state,p);
            }
            return ret;
         }
      }
      else
         return NO_MORE_TARGETS;
   }
}

static char *make_get_token(RvR_make_state *state, char **p)
{
   //TODO: move this bad boy if possible
   static char tmp[2000];

   skip_white(state,p);
   if(**p)
   {
      if(*p[0] == ']' || *p[0] == '[' || *p[0] == '=')
      {
         tmp[0] = *p[0];
         tmp[1] = 0;
         (*p)++;
         return buf_alloc(tmp, 2);
      }
      else if(*p[0] == '"')
      {
         int len = 0;
         char *c = tmp;
         (*p)++;

         while(**p && **p != '"')
         {
            *(c++) = *((*p)++);
            len++;
         }
         *c = 0;

         (*p)++;
         return buf_alloc(tmp, len + 1);


      }
      else if(token_char(**p))
      {
         int len = 0;
         char *c = tmp;

         while(**p && token_char(**p))
         {
            *(c++) = *((*p)++);
            len++;
         }
         *c = 0;

         return buf_alloc(tmp, len + 1);
      }
      else
      {
         make_error(state,"unexpected char '%c'", *p[0]);
         return 0;
      }
   }
   else
      return 0;

}

static char skip_white(RvR_make_state *state, char **p)
{
   while(**p
         && (**p == ' ' || **p == '\n' || **p == '\t' || **p == '\r'
             || **p == '#' || **p == 28))
   {
      if(**p == '#')
      {
         while(**p && **p != '\n')
            (*p)++;
      }
      else if(**p == ' ' || **p == '\r' || **p == '\t' || **p == 28)
         (*p)++;
      else if(**p == '\n')
      {
         (*p)++;
         state->line_on++;
      }
      else
      {
         char c = **p;
         (*p)++;
         return c;
      }
   }
   return 0;
}

static char *buf_alloc(char *p, int len)
{
   if(!buf || boff + len > BUF_SIZE)
   {
      buf = (char *)malloc(BUF_SIZE);
      boff = 0;
   }

   memcpy(buf + boff, p, len);
   boff += len;
   return buf + boff - len;
}

static void next_line(RvR_make_state *state, char **p)
{
   int l = state->line_on;
   while(l == state->line_on)
   {
      skip_white(state,p);
      if(l == state->line_on)
         get_token(state,p);
   }
}

static char *form(char *format, ...)
{
   va_list args;
   char buf[1024];

   va_start(args, form);
   vsprintf(buf, form, args);
   va_end(args);

   int len = strlen(buf);
   char *out = malloc(len+1);
   memcpy(out,buf,len+1);

   return out;
}

static char *load_file(RvR_make_state *state, char *fn, int set_current_file)
{
   FILE *fp = fopen(fn, "rb");
   if(!fp)
      return NULL;

   fseek(fp, 0, SEEK_END);
   int fsize = ftell(fp);
   fseek(fp, 0, SEEK_SET);

   char *buf = malloc(fsize + 1);
   fread(buf, fsize, 1, fp);
   buf[fsize] = 0;

   fclose(fp);

   if(set_current_file)
   {
      state->file_on = fn;
      state->line_on = 0;
      state->file_contents = buf;
   }

   return buf;
}

static void make_error(RvR_make_state *state, char *format, ...)
{
   if(state->file_on)
      fprintf(stderr, "%s:%d: ", state->file_on, state->line_on);

   va_list args;

   va_start(args, format);
   vfprintf(stderr, format, args);
   va_end(args);

   exit(1);
}

static void print_help(char **argv)
{
   printf("%s usage:\n"
           "%s --in filename --out filename --pal filename [OPTIONS]\n"
           "   -i, --in          input texture path\n"
           "   -o, --out         output texture path\n"
           "   -p, --pal         palette to use for assigning indices (.pal,.png,.hex,.gpl\n"
           "   -w, --wall        flag sprite as wall texture\n"
           "   -s, --sprite      flag sprite as sprite texture\n",
           argv[0], argv[0]);
}
