/*
 * Copyright (c) 2005-2010 Atheme Development Group
 * Rights to this code are as documented in doc/LICENSE.
 *
 * Controls REGNOLIMIT setting.
 */

#include "atheme.h"
#include "list_common.h"
#include "list.h"

static void ns_cmd_regnolimit(sourceinfo_t *si, int parc, char *parv[]);

struct command ns_regnolimit = { "REGNOLIMIT", N_("Allow a user to bypass registration limits."),
		      PRIV_ADMIN, 2, ns_cmd_regnolimit, { .path = "nickserv/regnolimit" } };

static bool has_regnolimit(const mynick_t *mn, const void *arg)
{
	myuser_t *mu = mn->owner;

	return ( mu->flags & MU_REGNOLIMIT ) == MU_REGNOLIMIT;
}

static void
mod_init(module_t *const restrict m)
{
	service_named_bind_command("nickserv", &ns_regnolimit);

	use_nslist_main_symbols(m);

	static list_param_t regnolimit;
	regnolimit.opttype = OPT_BOOL;
	regnolimit.is_match = has_regnolimit;

	list_register("regnolimit", &regnolimit);
}

static void
mod_deinit(const module_unload_intent_t intent)
{
	service_named_unbind_command("nickserv", &ns_regnolimit);

	list_unregister("regnolimit");
}

static void ns_cmd_regnolimit(sourceinfo_t *si, int parc, char *parv[])
{
	char *target = parv[0];
	char *action = parv[1];
	myuser_t *mu;

	if (!target || !action)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "REGNOLIMIT");
		command_fail(si, fault_needmoreparams, _("Usage: REGNOLIMIT <account> <ON|OFF>"));
		return;
	}

	if (!(mu = myuser_find_ext(target)))
	{
		command_fail(si, fault_nosuch_target, _("\2%s\2 is not registered."), target);
		return;
	}

	if (!strcasecmp(action, "ON"))
	{
		if (mu->flags & MU_REGNOLIMIT)
		{
			command_fail(si, fault_badparams, _("\2%s\2 can already bypass registration limits."), entity(mu)->name);
			return;
		}

		mu->flags |= MU_REGNOLIMIT;

		wallops("%s set the REGNOLIMIT option for the account \2%s\2.", get_oper_name(si), entity(mu)->name);
		logcommand(si, CMDLOG_ADMIN, "REGNOLIMIT:ON: \2%s\2", entity(mu)->name);
		command_success_nodata(si, _("\2%s\2 can now bypass registration limits."), entity(mu)->name);
	}
	else if (!strcasecmp(action, "OFF"))
	{
		if (!(mu->flags & MU_REGNOLIMIT))
		{
			command_fail(si, fault_badparams, _("\2%s\2 cannot bypass registration limits."), entity(mu)->name);
			return;
		}

		mu->flags &= ~MU_REGNOLIMIT;

		wallops("%s removed the REGNOLIMIT option on the account \2%s\2.", get_oper_name(si), entity(mu)->name);
		logcommand(si, CMDLOG_ADMIN, "REGNOLIMIT:OFF: \2%s\2", entity(mu)->name);
		command_success_nodata(si, _("\2%s\2 cannot bypass registration limits anymore."), entity(mu)->name);
	}
	else
	{
		command_fail(si, fault_needmoreparams, STR_INVALID_PARAMS, "REGNOLIMIT");
		command_fail(si, fault_needmoreparams, _("Usage: REGNOLIMIT <account> <ON|OFF>"));
	}
}

SIMPLE_DECLARE_MODULE_V1("nickserv/regnolimit", MODULE_UNLOAD_CAPABILITY_OK)
