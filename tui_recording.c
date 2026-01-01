/*
  +----------------------------------------------------------------------+
  | ext-tui: Screen recording PHP bindings                              |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "tui_internal.h"
#include "src/recording/recorder.h"

/* Resource name for recordings */
#define TUI_RECORDING_RES_NAME "TuiRecording"

/* Resource type ID - declared extern in tui_internal.h */
int le_tui_recording;

/* {{{ Resource destructor */
void tui_recording_dtor(zend_resource *res)
{
    tui_recording *rec = (tui_recording *)res->ptr;
    if (rec) {
        tui_recording_free(rec);
    }
}
/* }}} */

/* {{{ proto resource tui_record_create(int $width, int $height, ?string $title = null)
   Create a new recording context */
PHP_FUNCTION(tui_record_create)
{
    zend_long width, height;
    zend_string *title = NULL;

    ZEND_PARSE_PARAMETERS_START(2, 3)
        Z_PARAM_LONG(width)
        Z_PARAM_LONG(height)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(title)
    ZEND_PARSE_PARAMETERS_END();

    tui_recording *rec = tui_recording_create(
        (int)width, (int)height,
        title ? ZSTR_VAL(title) : NULL);

    if (!rec) {
        zend_throw_exception(tui_resource_exception_ce,
            "Failed to create recording", 0);
        RETURN_THROWS();
    }

    RETURN_RES(zend_register_resource(rec, le_tui_recording));
}
/* }}} */

/* {{{ proto bool tui_record_start(resource $recording)
   Start or resume recording */
PHP_FUNCTION(tui_record_start)
{
    zval *zrec;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zrec)
    ZEND_PARSE_PARAMETERS_END();

    tui_recording *rec = (tui_recording *)zend_fetch_resource(
        Z_RES_P(zrec), TUI_RECORDING_RES_NAME, le_tui_recording);

    if (!rec) {
        RETURN_FALSE;
    }

    RETURN_BOOL(tui_recording_start(rec) == 0);
}
/* }}} */

/* {{{ proto bool tui_record_pause(resource $recording)
   Pause recording */
PHP_FUNCTION(tui_record_pause)
{
    zval *zrec;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zrec)
    ZEND_PARSE_PARAMETERS_END();

    tui_recording *rec = (tui_recording *)zend_fetch_resource(
        Z_RES_P(zrec), TUI_RECORDING_RES_NAME, le_tui_recording);

    if (!rec) {
        RETURN_FALSE;
    }

    RETURN_BOOL(tui_recording_pause(rec) == 0);
}
/* }}} */

/* {{{ proto bool tui_record_resume(resource $recording)
   Resume paused recording */
PHP_FUNCTION(tui_record_resume)
{
    zval *zrec;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zrec)
    ZEND_PARSE_PARAMETERS_END();

    tui_recording *rec = (tui_recording *)zend_fetch_resource(
        Z_RES_P(zrec), TUI_RECORDING_RES_NAME, le_tui_recording);

    if (!rec) {
        RETURN_FALSE;
    }

    RETURN_BOOL(tui_recording_resume(rec) == 0);
}
/* }}} */

/* {{{ proto bool tui_record_stop(resource $recording)
   Stop recording */
PHP_FUNCTION(tui_record_stop)
{
    zval *zrec;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zrec)
    ZEND_PARSE_PARAMETERS_END();

    tui_recording *rec = (tui_recording *)zend_fetch_resource(
        Z_RES_P(zrec), TUI_RECORDING_RES_NAME, le_tui_recording);

    if (!rec) {
        RETURN_FALSE;
    }

    RETURN_BOOL(tui_recording_stop(rec) == 0);
}
/* }}} */

/* {{{ proto bool tui_record_capture(resource $recording, string $data)
   Capture a frame of terminal output */
PHP_FUNCTION(tui_record_capture)
{
    zval *zrec;
    zend_string *data;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zrec)
        Z_PARAM_STR(data)
    ZEND_PARSE_PARAMETERS_END();

    tui_recording *rec = (tui_recording *)zend_fetch_resource(
        Z_RES_P(zrec), TUI_RECORDING_RES_NAME, le_tui_recording);

    if (!rec) {
        RETURN_FALSE;
    }

    RETURN_BOOL(tui_recording_capture_frame(rec, ZSTR_VAL(data), ZSTR_LEN(data)) == 0);
}
/* }}} */

/* {{{ proto float tui_record_duration(resource $recording)
   Get recording duration in seconds */
PHP_FUNCTION(tui_record_duration)
{
    zval *zrec;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zrec)
    ZEND_PARSE_PARAMETERS_END();

    tui_recording *rec = (tui_recording *)zend_fetch_resource(
        Z_RES_P(zrec), TUI_RECORDING_RES_NAME, le_tui_recording);

    if (!rec) {
        RETURN_DOUBLE(0.0);
    }

    RETURN_DOUBLE(tui_recording_duration(rec));
}
/* }}} */

/* {{{ proto int tui_record_frame_count(resource $recording)
   Get number of frames in recording */
PHP_FUNCTION(tui_record_frame_count)
{
    zval *zrec;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zrec)
    ZEND_PARSE_PARAMETERS_END();

    tui_recording *rec = (tui_recording *)zend_fetch_resource(
        Z_RES_P(zrec), TUI_RECORDING_RES_NAME, le_tui_recording);

    if (!rec) {
        RETURN_LONG(0);
    }

    RETURN_LONG(rec->frame_count);
}
/* }}} */

/* {{{ proto string tui_record_export(resource $recording)
   Export recording as asciicast v2 JSON */
PHP_FUNCTION(tui_record_export)
{
    zval *zrec;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zrec)
    ZEND_PARSE_PARAMETERS_END();

    tui_recording *rec = (tui_recording *)zend_fetch_resource(
        Z_RES_P(zrec), TUI_RECORDING_RES_NAME, le_tui_recording);

    if (!rec) {
        RETURN_EMPTY_STRING();
    }

    char *data;
    size_t len;

    if (tui_recording_to_asciicast(rec, &data, &len) < 0) {
        RETURN_EMPTY_STRING();
    }

    RETVAL_STRINGL(data, len);
    free(data);
}
/* }}} */

/* {{{ proto bool tui_record_save(resource $recording, string $path)
   Save recording to file as asciicast v2 format */
PHP_FUNCTION(tui_record_save)
{
    zval *zrec;
    zend_string *path;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zrec)
        Z_PARAM_STR(path)
    ZEND_PARSE_PARAMETERS_END();

    tui_recording *rec = (tui_recording *)zend_fetch_resource(
        Z_RES_P(zrec), TUI_RECORDING_RES_NAME, le_tui_recording);

    if (!rec) {
        RETURN_FALSE;
    }

    RETURN_BOOL(tui_recording_save(rec, ZSTR_VAL(path)) == 0);
}
/* }}} */

/* {{{ proto void tui_record_destroy(resource $recording)
   Destroy recording resource and free memory */
PHP_FUNCTION(tui_record_destroy)
{
    zval *zrec;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zrec)
    ZEND_PARSE_PARAMETERS_END();

    tui_recording *rec = (tui_recording *)zend_fetch_resource(
        Z_RES_P(zrec), TUI_RECORDING_RES_NAME, le_tui_recording);

    if (rec) {
        zend_list_close(Z_RES_P(zrec));
    }
}
/* }}} */
