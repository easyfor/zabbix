/*
** Zabbix
** Copyright (C) 2001-2019 Zabbix SIA
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**/

#include "zbxmocktest.h"
#include "zbxmockdata.h"
#include "zbxmockassert.h"
#include "zbxmockutil.h"

#include "common.h"
#include "zbxalgo.h"
#include "module.h"
#include "sysinfo.h"


static int	zbx_mock_str_to_key_access_type(const char *str)
{
	if (0 == strcmp(str, "ZBX_KEY_ACCESS_ALLOW"))
		return ZBX_KEY_ACCESS_ALLOW;

	if (0 == strcmp(str, "ZBX_KEY_ACCESS_DENY"))
		return ZBX_KEY_ACCESS_DENY;

	fail_msg("Unknown key access type \"%s\"", str);
	return ZBX_KEY_ACCESS_ALLOW;
}

void	zbx_mock_test_entry(void **state)
{
	zbx_mock_handle_t	hrules, hrule, hmetrics, hmetric;
	const char		*type, *pattern, *key;
	int			expected_ret, actual_ret;

	hrules = zbx_mock_get_parameter_handle("in.rules");
	init_key_access_rules();

	while (ZBX_MOCK_SUCCESS == zbx_mock_vector_element(hrules, &hrule))
	{
		type = zbx_mock_get_object_member_string(hrule, "type");
		pattern = zbx_mock_get_object_member_string(hrule, "pattern");
		if (SUCCEED != add_key_access_rule(pattern, zbx_mock_str_to_key_access_type(type)))
		{
			free_key_access_rules();
			fail_msg("Bad key access rule definition");
		}
	}

	finalize_key_access_rules_configuration();
	hmetrics = zbx_mock_get_parameter_handle("out.metrics");

	while (ZBX_MOCK_SUCCESS == zbx_mock_vector_element(hmetrics, &hmetric))
	{
		key = zbx_mock_get_object_member_string(hmetric, "metric");
		expected_ret = zbx_mock_str_to_key_access_type(zbx_mock_get_object_member_string(hmetric, "result"));

		actual_ret = check_key_access_rules(key);

		if (expected_ret != actual_ret)
			fail_msg("Unexpected result for metric \"%s\": %s, expected %s", key,
					actual_ret == ZBX_KEY_ACCESS_ALLOW ? "Allow" : "Deny",
					expected_ret == ZBX_KEY_ACCESS_ALLOW ? "Allow" : "Deny");
	}

	free_key_access_rules();
}
