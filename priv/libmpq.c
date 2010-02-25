#include "erl_nif.h"
#include "libmpq/mpq.h"

#define MPQ_UNIT32_T(name, func)                                        \
  static ERL_NIF_TERM name(ErlNifEnv* env, ERL_NIF_TERM mpq_archive_t) { \
    return nif_mpq_uint32_t(env, mpq_archive_t, (func));                \
  }

#define MPQ_OFF_T(name, func)                                           \
  static ERL_NIF_TERM name(ErlNifEnv* env, ERL_NIF_TERM mpq_archive_t) { \
    return nif_mpq_off_t(env, mpq_archive_t, (func));                   \
  }

#define READ_MPQ_ARCHIVE()                                \
  unsigned long mpq_archive;                              \
  if (!enif_get_ulong(env, mpq_archive_t, &mpq_archive))  \
  {                                                       \
    return enif_make_badarg(env);                         \
  }

#define READ_FILE_NUMBER()                                      \
  uint32_t file_number;                                         \
  if (!enif_get_int(env, file_number_t, (int *) &file_number))  \
  {                                                             \
    return enif_make_badarg(env);                               \
  }
   

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

// func that operates on mpq and returns uint32_t
static ERL_NIF_TERM nif_mpq_uint32_t(ErlNifEnv* env, ERL_NIF_TERM mpq_archive_t, int32_t (*f)(mpq_archive_s *, uint32_t *))
{
  READ_MPQ_ARCHIVE();
  uint32_t result;
  if (!f((mpq_archive_s *)mpq_archive, &result))
  {
    return my_enif_make_error(env, "Error performing operation");
  }
  return enif_make_tuple(env, 2,
                         enif_make_atom(env, "ok"),
                         enif_make_int(env, result));
}

// func that operates on mpq and returns libmpq__off_t
static ERL_NIF_TERM nif_mpq_off_t(ErlNifEnv* env, ERL_NIF_TERM mpq_archive_t, int32_t (*f)(mpq_archive_s *, libmpq__off_t *))
{
  READ_MPQ_ARCHIVE();
  libmpq__off_t result;
  if (!f((mpq_archive_s *)mpq_archive, &result))
  {
    return my_enif_make_error(env, "Error performing operation");
  }
  return enif_make_tuple(env, 2,
                         enif_make_atom(env, "ok"),
                         enif_make_ulong(env, result));
}

// func that operats on a mpq file and returns int32_t
static ERL_NIF_TERM nif_mpq_file_uint32_t(ErlNifEnv* env, ERL_NIF_TERM mpq_archive_t, ERL_NIF_TERM file_number_t, int32_t (*f)(mpq_archive_s *, uint32_t, uint32_t *))
{
  READ_MPQ_ARCHIVE();
  READ_FILE_NUMBER();
  uint32_t result;
  if (!f((mpq_archive_s *)mpq_archive, file_number, &result))
  {
    return my_enif_make_error(env, "Error performing operation");
  }
  return enif_make_tuple(env, 2,
                         enif_make_atom(env, "ok"),
                         enif_make_int(env, result));
}

// func that operats on mpq file and returns libmpq__off_t
static ERL_NIF_TERM nif_mpq_file_off_t(ErlNifEnv* env, ERL_NIF_TERM mpq_archive_t, ERL_NIF_TERM file_number_t, int32_t (*f)(mpq_archive_s *, uint32_t, libmpq__off_t *))
{
  READ_MPQ_ARCHIVE();
  READ_FILE_NUMBER();
  libmpq__off_t result;
  if (!f((mpq_archive_s *)mpq_archive, file_number, &result))
  {
    return my_enif_make_error(env, "Error performing operation");
  }
  return enif_make_tuple(env, 2,
                         enif_make_atom(env, "ok"),
                         enif_make_ulong(env, result));
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

static ERL_NIF_TERM nif_archive_close(ErlNifEnv* env, ERL_NIF_TERM mpq_archive_t)
{
  READ_MPQ_ARCHIVE();
  if(!libmpq__archive_close((mpq_archive_s*)mpq_archive))
  {
    return my_enif_make_error(env, "Error closing archive");
  }
  return enif_make_atom(env, "ok");
}

MPQ_OFF_T(nif_archive_packed_size,libmpq__archive_packed_size);
MPQ_OFF_T(nif_archive_unpacked_size,libmpq__archive_unpacked_size);
MPQ_OFF_T(nif_archive_offset,libmpq__archive_offset);
MPQ_UNIT32_T(nif_archive_version,libmpq__archive_version);
MPQ_UNIT32_T(nif_archive_files,libmpq__archive_files);


static ErlNifFunc nif_funcs[] = 
  {
    {"archive_open", 2, (void*)nif_archive_open},
    {"archive_close", 1, (void*)nif_archive_close},
    {"archive_packed_size",1,(void*)nif_archive_packed_size},
    {"archive_unpacked_size",1,(void*)nif_archive_unpacked_size},
    {"archive_offset",1,(void*)nif_archive_offset},
    {"archive_version",1,(void*)nif_archive_version},
    {"archive_files",1,(void*)nif_archive_files}
  };
ERL_NIF_INIT(libmpq,nif_funcs,NULL,NULL,NULL,NULL)
