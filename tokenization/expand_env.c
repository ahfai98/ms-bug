#include "minishell.h"

static int	find_env_end(char *env_start)
{
	int	i;

	if (env_start[0] == '$')
		return (-1);
	if (!ft_isalpha(env_start[0]) && env_start[0] != '_')
		return (0);
	i = 1;
	while (env_start[i])
	{
		if (!ft_isalnum(env_start[i]) && env_start[i] != '_')
			break ;
		i ++;
	}
	return (i);
}

static int	expand_env_iter(int *env_end, char *env_start, char **buffer, t_token_info *token_info)
{
	char	*var_name;
	char	*brace;

	if (find_env_end(env_start) < 0)
		return (ft_rstrjoin(buffer, "$"), 0);
	*env_end = find_env_end(env_start);
	if (!ft_isalpha(*env_start))
		*env_end = 1;
	var_name = ft_substr(env_start - 1, 0, *env_end + 1);
	if (get_env_var(var_name + 1, &token_info->global_env))
		ft_rstrjoin(buffer, get_env_var(var_name + 1,
				&token_info->global_env)->value);
	free(var_name);
	return (0);
}

static int last_exit_status(char **buffer)
{
	ft_rstrjoin(buffer, "exit_status");
	return 1;
}

// returns a string with any occurences of $<text> expanded
char	*expand_env(char *string, t_token_info *token_info)
{
	char	*buffer;
	char	*i_env_start;
	char	*i_before;
	int		i_env_end;

	i_env_start = 0;
	i_env_end = 0;
	i_before = string;
	if (!ft_strchr(string, '$'))
		return (string);
	buffer = ft_strdup("");
	i_env_start = ft_strchr(string, '$') + 1;
	while (*i_env_start)
	{
		ft_rstrjoin(&buffer, ft_substr(i_before, 0,
				i_env_start - i_before - 1));
		if (!strncmp("?", i_env_start, 1))
			i_env_end += last_exit_status(&buffer);
		else
			expand_env_iter(&i_env_end, i_env_start, &buffer, token_info);
		i_before = i_env_start + i_env_end;
		if (!ft_strchr(i_env_start, '$'))
			break ;
		i_env_start = ft_strchr(i_env_start, '$') + 1;
	}
	ft_rstrjoin(&buffer, ft_substr(i_before, 0, ft_strlen(i_before)));
	return (buffer);
}
