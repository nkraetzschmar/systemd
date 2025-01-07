/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <unistd.h>

#include "json-util.h"
#include "varlink-io.systemd.service.h"

static SD_VARLINK_DEFINE_METHOD(Ping);

static SD_VARLINK_DEFINE_METHOD(Reload);

static SD_VARLINK_DEFINE_METHOD(
                SetLogLevel,
                SD_VARLINK_FIELD_COMMENT("The maximum log level, using BSD syslog log level integers."),
                SD_VARLINK_DEFINE_INPUT(level, SD_VARLINK_INT, SD_VARLINK_NULLABLE));

SD_VARLINK_DEFINE_INTERFACE(
                io_systemd_service,
                "io.systemd.service",
                SD_VARLINK_INTERFACE_COMMENT("An interface to control basic properties of systemd services."),
                SD_VARLINK_SYMBOL_COMMENT("Checks if the service is running."),
                &vl_method_Ping,
                SD_VARLINK_SYMBOL_COMMENT("Reloads configuration files."),
                &vl_method_Reload,
                SD_VARLINK_SYMBOL_COMMENT("Sets the maximum log level."),
                &vl_method_SetLogLevel);

/* Generic implementations for some of the method calls above */

int varlink_method_ping(sd_varlink *link, sd_json_variant *parameters, sd_varlink_method_flags_t flags, void *userdata) {
        int r;

        assert(link);

        r = sd_varlink_dispatch(link, parameters, /* dispatch_table= */ NULL, /* userdata= */ NULL);
        if (r != 0)
                return r;

        log_debug("Received io.systemd.service.Ping");

        return sd_varlink_reply(link, NULL);
}

int varlink_method_set_log_level(sd_varlink *link, sd_json_variant *parameters, sd_varlink_method_flags_t flags, void *userdata) {
        static const sd_json_dispatch_field dispatch_table[] = {
                { "level", _SD_JSON_VARIANT_TYPE_INVALID, json_dispatch_log_level, 0, SD_JSON_MANDATORY },
                {}
        };

        int r, level;
        uid_t uid;

        assert(link);
        assert(parameters);

        r = sd_varlink_dispatch(link, parameters, dispatch_table, &level);
        if (r != 0)
                return r;

        r = sd_varlink_get_peer_uid(link, &uid);
        if (r < 0)
                return r;

        if (uid != 0 && uid != getuid())
                return sd_varlink_error(link, SD_VARLINK_ERROR_PERMISSION_DENIED, parameters);

        log_debug("Received io.systemd.service.SetLogLevel(%i)", level);

        log_set_max_level(level);

        return sd_varlink_reply(link, NULL);
}
