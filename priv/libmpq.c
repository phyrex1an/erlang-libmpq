#include "erl_nif.h"
#include "libmpq/mpq.h"

#define MPQ_UINT32_T(name, func)                                        \
  static ERL_NIF_TERM name(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) { \
    return nif_mpq_uint32_t(env, argc, argv, (func));                   \
  }

#define MPQ_OFF_T(name, func)                                           \
  static ERL_NIF_TERM name(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) { \
    return nif_mpq_off_t(env, argc, argv, (func));                           \
  }

#define MPQ_FILE_UINT32_T(name, func)                                   \
  static ERL_NIF_TERM name(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) \
  {                                                                     \
    return nif_mpq_file_uint32_t(env, argc, argv, (func));              \
  }

#define MPQ_FILE_OFF_T(name, func)                                      \
  static ERL_NIF_TERM name(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) \
  {                                                                     \
    return nif_mpq_file_off_t(env, argc, argv, (func));                 \
  }

#define READ_MPQ_ARCHIVE(pos)                                           \
  mpq_archive_s *mpq_archive;                                           \
  if (!enif_get_ulong(env, argv[pos], (unsigned long *) &mpq_archive))  \
  {                                                                     \
    return enif_make_badarg(env);                                       \
  }

#define READ_FILE_NUMBER(pos)                                   \
  uint32_t file_number;                                         \
  if (!enif_get_int(env, argv[pos], (int *) &file_number))      \
  {                                                             \
    return enif_make_badarg(env);                               \
  }
   

static ERL_NIF_TERM my_enif_make_error(ErlNifEnv *env, char *msg)
{
  return enif_make_tuple(env, 2,
                         enif_make_atom(env, "error"),
                         enif_make_string(env, msg, ERL_NIF_LATIN1));
}

static ERL_NIF_TERM my_enif_make_error_code(ErlNifEnv *env, char *msg, int code)
{
  return enif_make_tuple(env, 3,
                         enif_make_atom(env, "error"),
                         enif_make_string(env, msg, ERL_NIF_LATIN1),
                         enif_make_int(env, code));
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
  int size=my_enif_list_size(env, list);

  if (!(buf = (char*) enif_alloc(env, size+1)))
  {
    return NULL;
  }
  if (enif_get_string(env, list, buf, size+1, ERL_NIF_LATIN1)<1)
  {
    enif_free(env, buf);
    return NULL;
  }
  return buf;
}

// func that operates on mpq and returns uint32_t
static ERL_NIF_TERM nif_mpq_uint32_t(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[], int32_t (*f)(mpq_archive_s *, uint32_t *))
{
  READ_MPQ_ARCHIVE(0);
  uint32_t result;
  if (f(mpq_archive, &result))
  {
    return my_enif_make_error(env, "Error performing operation");
  }
  return enif_make_tuple(env, 2,
                         enif_make_atom(env, "ok"),
                         enif_make_int(env, result));
}

// func that operates on mpq and returns libmpq__off_t
static ERL_NIF_TERM nif_mpq_off_t(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[], int32_t (*f)(mpq_archive_s *, libmpq__off_t *))
{
  READ_MPQ_ARCHIVE(0);
  libmpq__off_t result;
  if (f(mpq_archive, &result))
  {
    return my_enif_make_error(env, "Error performing operation");
  }
  return enif_make_tuple(env, 2,
                         enif_make_atom(env, "ok"),
                         enif_make_ulong(env, result));
}

// func that operats on a mpq file and returns int32_t
static ERL_NIF_TERM nif_mpq_file_uint32_t(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[], int32_t (*f)(mpq_archive_s *, uint32_t, uint32_t *))
{
  READ_MPQ_ARCHIVE(0);
  READ_FILE_NUMBER(1);
  uint32_t result;
  if (f(mpq_archive, file_number, &result))
  {
    return my_enif_make_error(env, "Error performing operation");
  }
  return enif_make_tuple(env, 2,
                         enif_make_atom(env, "ok"),
                         enif_make_int(env, result));
}

// func that operats on mpq file and returns libmpq__off_t
static ERL_NIF_TERM nif_mpq_file_off_t(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[], int32_t (*f)(mpq_archive_s *, uint32_t, libmpq__off_t *))
{
  READ_MPQ_ARCHIVE(0);
  READ_FILE_NUMBER(1);
  libmpq__off_t result;
  if (f(mpq_archive, file_number, &result))
  {
    return my_enif_make_error(env, "Error performing operation");
  }
  return enif_make_tuple(env, 2,
                         enif_make_atom(env, "ok"),
                         enif_make_ulong(env, result));
}

static ERL_NIF_TERM nif_archive_open(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  char *mpq_filename = my_enif_get_string(env, argv[0]);
  if (!mpq_filename)
  {
    return enif_make_badarg(env);
  }

  unsigned long archive_offset;

  if (!enif_get_ulong(env, argv[1], &archive_offset))
  {
    enif_free(env, mpq_filename);
    return enif_make_badarg(env);
  }

  mpq_archive_s *mpq_archive;
  int32_t result = libmpq__archive_open(&mpq_archive, mpq_filename, archive_offset);
  enif_free(env, mpq_filename);
  if (result)
  {
    return my_enif_make_error_code(env, "Error opening archive", result);
  }
  return enif_make_tuple(env, 2,
                         enif_make_atom(env, "ok"),
                         enif_make_ulong(env, (unsigned long)mpq_archive));
}

static ERL_NIF_TERM nif_archive_close(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  READ_MPQ_ARCHIVE(0);
  if(libmpq__archive_close(mpq_archive))
  {
    return my_enif_make_error(env, "Error closing archive");
  }
  return enif_make_atom(env, "ok");
}

MPQ_OFF_T(nif_archive_packed_size,libmpq__archive_packed_size);
MPQ_OFF_T(nif_archive_unpacked_size,libmpq__archive_unpacked_size);
MPQ_OFF_T(nif_archive_offset,libmpq__archive_offset);
MPQ_UINT32_T(nif_archive_version,libmpq__archive_version);
MPQ_UINT32_T(nif_archive_files,libmpq__archive_files);


MPQ_FILE_OFF_T(nif_file_packed_size, libmpq__file_packed_size);
MPQ_FILE_OFF_T(nif_file_unpacked_size, libmpq__file_unpacked_size);
MPQ_FILE_OFF_T(nif_file_offset, libmpq__file_offset);
MPQ_FILE_UINT32_T(nif_file_blocks,libmpq__file_blocks);
MPQ_FILE_UINT32_T(nif_file_encrypted,libmpq__file_encrypted);
MPQ_FILE_UINT32_T(nif_file_compressed,libmpq__file_compressed);
MPQ_FILE_UINT32_T(nif_file_imploded,libmpq__file_imploded);

static ERL_NIF_TERM nif_file_number(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  READ_MPQ_ARCHIVE(0);
  char *filename = my_enif_get_string(env, argv[1]);
  if(!filename)
  {
    return enif_make_badarg(env);
  }
  uint32_t number;
  int32_t result = libmpq__file_number(mpq_archive, filename, &number);
  enif_free(env, filename);
  if (result)
  {
    return my_enif_make_error(env, "Failed to get file number");
  }
  return enif_make_tuple(env, 2,
                         enif_make_atom(env, "ok"),
                         enif_make_int(env, number));
}

static ERL_NIF_TERM nif_file_read(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  READ_MPQ_ARCHIVE(0);
  READ_FILE_NUMBER(1);
  libmpq__off_t unpacked_size;
  if(libmpq__file_unpacked_size(mpq_archive, file_number, &unpacked_size))
  {
    return my_enif_make_error(env, "Error getting file size");
  }
  
  ErlNifBinary bin;
  libmpq__off_t transferred;
  enif_alloc_binary(env, unpacked_size, &bin);
  if (libmpq__file_read((mpq_archive_s *)mpq_archive, file_number, bin.data, unpacked_size, &transferred))
  {
    enif_release_binary(env, &bin);
    return my_enif_make_error(env, "Error reading file content");
  }
  return enif_make_tuple(env, 3,
                         enif_make_atom(env, "ok"),
                         enif_make_binary(env, &bin),
                         enif_make_ulong(env, transferred));
}

static ErlNifFunc nif_funcs[] = 
  {
    {"archive_open", 2, (void*)nif_archive_open},
    {"archive_close", 1, (void*)nif_archive_close},
    {"archive_packed_size",1,(void*)nif_archive_packed_size},
    {"archive_unpacked_size",1,(void*)nif_archive_unpacked_size},
    {"archive_offset",1,(void*)nif_archive_offset},
    {"archive_version",1,(void*)nif_archive_version},
    {"archive_files",1,(void*)nif_archive_files},

    {"file_packed_size",2,(void*)nif_file_packed_size},
    {"file_unpacked_size",2,(void*)nif_file_unpacked_size},
    {"file_offset",2,(void*)nif_file_offset},
    {"file_blocks",2,(void*)nif_file_blocks},
    {"file_encrypted",2,(void*)nif_file_encrypted},
    {"file_compressed",2,(void*)nif_file_compressed},
    {"file_imploded",2,(void*)nif_file_imploded},
    {"file_number",2,(void*)nif_file_number},
    {"file_read",2,(void*)nif_file_read}

  };
ERL_NIF_INIT(libmpq,nif_funcs,NULL,NULL,NULL,NULL)
