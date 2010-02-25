-module(libmpq).
-on_load(init/0).
-export([init/0]).

-export([archive_open/2,
         archive_close/1,
         archive_packed_size/1,
         archive_unpacked_size/1,
         archive_offset/1,
         archive_version/1,
         archive_files/1
        ]).

-export([file_packed_size/2,
         file_unpacked_size/2,
         file_offset/2,
         file_blocks/2,
         file_encrypted/2,
         file_compressed/2,
         file_imploded/2,
         file_number/2,
         file_read/2
        ]).

init() ->
    ok = erlang:load_nif("/home/phyrex1an/git/erlang-libmpq/priv/libmpq", 0),
    true.

%% {ok, MPQ = long()}
archive_open(_MpqFilename, _ArchiveOffset) ->
    nif_error().

archive_close(_MpqArchive) ->
    nif_error().

archive_packed_size(_MpqArchive) ->
    nif_error().

archive_unpacked_size(_MpqArchive) ->
    nif_error().

archive_offset(_MpqArchive) ->
    nif_error().

archive_version(_MpqArchive) ->
    nif_error().

archive_files(_MpqArchive) ->
    nif_error().

file_packed_size(_MpqArchive, _FileNumber) ->
    nif_error().

file_unpacked_size(_MpqArchive, _FileNumber) ->
    nif_error().

file_offset(_MpqArchive, _FileNumber) ->
    nif_error().

file_blocks(_MpqArchive, _FileNumber) ->
    nif_error().

file_encrypted(_MpqArchive, _FileNumber) ->
    nif_error().

file_compressed(_MpqArchive, _FileNumber) ->
    nif_error().

file_imploded(_MpqArchive, _FileNumber) ->
    nif_error().

file_number(_MpqArchive, _FileName) ->
    nif_error().

file_read(_MpqArchive, _FileNumber) ->
    nif_error().

nif_error() ->
    {error, "NIF not loaded yet"}.
