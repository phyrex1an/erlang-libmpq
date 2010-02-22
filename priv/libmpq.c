#include "erl_nif.h"
#include "libmpq/mpq.h"

static ERL_NIF_TERM my_enif_make_error(ErlNifEnv *env, char *msg)
{
  return enif_make_tuple(env, 2,
			 enif_make_atom(env, "error"),
			 enif_make_string(env, msg));
}

static int my_enif_list_size(ErlNifEnv* env, ERL_NIF_TERM list)
{
  ERL_NIF_TERM head, tail, nexttail;
  int size = 0;
  tail = list;
  while(enif_get_list_cell(env, tail, &head, &nexttail))
  {
    tail = nexttail;
    size = size+1;
  }
  return size;
}

static char* my_enif_get_string(ErlNifEnv *env, ERL_NIF_TERM list)
{
  char *buf;
  ERL_NIF_TERM head, tail, nexttail;
  int size=my_enif_list_size(env, list);
  int cell;

  if (!(buf = (char*) enif_alloc(env, size+1)))
  {
    return NULL;
  }
  
  tail = list;
  size=0;
  while(enif_get_list_cell(env, tail, &head, &nexttail))
  {
    tail = nexttail;
    if (!enif_get_int(env, head, &cell)) 
    {
      enif_free(env, buf);
      return NULL;
    }
    buf[size] = (char)cell;
    size++;
  }
  
  buf[size]='\0';
  return buf;
}

static ERL_NIF_TERM nif_archive_open(ErlNifEnv* env, ERL_NIF_TERM mpq_filename_t, ERL_NIF_TERM archive_offset_t)
{
  char *mpq_filename = my_enif_get_string(env, mpq_filename_t);
  if (!mpq_filename)
  {
    return enif_make_badarg(env);
  }

  unsigned long archive_offset;

  if (!enif_get_ulong(env, archive_offset_t, &archive_offset))
  {
    enif_free(env, mpq_filename);
    return enif_make_badarg(env);
  }

  mpq_archive_s *mpq_archive;
  int32_t result = libmpq__archive_open(&mpq_archive, mpq_filename, archive_offset);
  enif_free(env, mpq_filename);
  if (!result)
  {
    return my_enif_make_error(env, "Error opening archive");
  }
  return enif_make_tuple(env, 2,
                         enif_make_atom(env, "ok"),
                         enif_make_ulong(env, (unsigned long)mpq_archive));
}



static ErlNifFunc nif_funcs[] = 
{
  {"archive_open", 2, (void*)nif_archive_open}
};
ERL_NIF_INIT(libmpq,nif_funcs,NULL,NULL,NULL,NULL)
