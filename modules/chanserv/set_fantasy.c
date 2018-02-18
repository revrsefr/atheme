/*
 * Copyright (c) 2003-2004 E. Will et al.
 * Copyright (c) 2006-2010 Atheme Development Group
 * Rights to this code are documented in doc/LICENSE.
 *
 * This file contains routines to handle the CService SET FANTASY command.
 */

#include "atheme.h"

static void cs_set_fantasy_config_ready(void *unused);

static void cs_cmd_set_fantasy(sourceinfo_t *si, int parc, char *parv[]);

struct command cs_set_fantasy = { "FANTASY", N_("Allows or disallows in-channel commands."), AC_NONE, 2, cs_cmd_set_fantasy, { .path = "cservice/set_fantasy" } };

mowgli_patricia_t **cs_set_cmdtree;

static void
mod_init(module_t *const restrict m)
{
	MODULE_TRY_REQUEST_SYMBOL(m, cs_set_cmdtree, "chanserv/set_core", "cs_set_cmdtree");

	command_add(&cs_set_fantasy, *cs_set_cmdtree);

	hook_add_event("config_ready");
	hook_add_config_ready(cs_set_fantasy_config_ready);
}

static void
mod_deinit(const module_unload_intent_t intent)
{
	command_delete(&cs_set_fantasy, *cs_set_cmdtree);

	hook_del_config_ready(cs_set_fantasy_config_ready);
}

static void cs_set_fantasy_config_ready(void *unused)
{
	if (chansvs.fantasy)
		cs_set_fantasy.access = NULL;
	else
		cs_set_fantasy.access = AC_DISABLED;
}

static void cs_cmd_set_fantasy(sourceinfo_t *si, int parc, char *parv[])
{
	mychan_t *mc;

	if (!(mc = mychan_find(parv[0])))
	{
		command_fail(si, fault_nosuch_target, _("Channel \2%s\2 is not registered."), parv[0]);
		return;
	}

	if (!parv[1])
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "SET FANTASY");
		return;
	}

	if (!chanacs_source_has_flag(mc, si, CA_SET))
	{
		command_fail(si, fault_noprivs, _("You are not authorized to perform this command."));
		return;
	}

	if (!strcasecmp("ON", parv[1]))
	{
		metadata_t *md = metadata_find(mc, "disable_fantasy");

		if (!md)
		{
			command_fail(si, fault_nochange, _("The \2%s\2 flag is already set for channel \2%s\2."), "FANTASY", mc->name);
			return;
		}

		metadata_delete(mc, "disable_fantasy");

		logcommand(si, CMDLOG_SET, "SET:FANTASY:ON: \2%s\2", mc->name);
		verbose(mc, _("\2%s\2 enabled the FANTASY flag"), get_source_name(si));
		command_success_nodata(si, _("The \2%s\2 flag has been set for channel \2%s\2."), "FANTASY", mc->name);
		return;
	}
	else if (!strcasecmp("OFF", parv[1]))
	{
		metadata_t *md = metadata_find(mc, "disable_fantasy");

		if (md)
		{
			command_fail(si, fault_nochange, _("The \2%s\2 flag is not set for channel \2%s\2."), "FANTASY", mc->name);
			return;
		}

		metadata_add(mc, "disable_fantasy", "on");

		logcommand(si, CMDLOG_SET, "SET:FANTASY:OFF: \2%s\2", mc->name);
		verbose(mc, _("\2%s\2 disabled the FANTASY flag"), get_source_name(si));
		command_success_nodata(si, _("The \2%s\2 flag has been removed for channel \2%s\2."), "FANTASY", mc->name);
		return;
	}
	else
	{
		command_fail(si, fault_badparams, STR_INVALID_PARAMS, "FANTASY");
		return;
	}
}

SIMPLE_DECLARE_MODULE_V1("chanserv/set_fantasy", MODULE_UNLOAD_CAPABILITY_OK)
