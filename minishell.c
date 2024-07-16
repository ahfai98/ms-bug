#include "minishell.h"


int	bash_cmd(char **env, t_token_info *token_info)
{
	char	**args;
	int		i;

	args = token_info->cmd_start->cmds;
	if ((!ft_strcmp(args[0], "echo")) && args[1] && !ft_strcmp(args[1], "-n"))
	{
		i = 1;
		while (args[++i])
			printf("%s", args[i]);
	}
	if (!ft_strcmp(args[0], "exit"))
	{
		free_TokenList(token_info);
		exit(0);
	}
	if (!ft_strcmp(args[0], "cd"))
		chdir(args[1]);
	if (!ft_strcmp(args[0], "env"))
		print_env(&token_info->global_env, 'e');
	if (!ft_strcmp(args[0], "unset") && args[1])
		unset_env(args[1], &token_info->global_env, token_info);
	if (!ft_strcmp(args[0], "export"))
		export(token_info);
	return (0);
}

void	ctrl_c_function(int signum)
{
	rl_replace_line("", 0);
	printf("\n");
	rl_on_new_line();
	rl_redisplay();
	signal(SIGINT, ctrl_c_function);
}

// an initial check to see if commands do exist
t_token	*scan_cmd(t_token_info *token_info)
{
	t_token		*list;

	list = token_info->token_chunks;
	token_info->cmd_start = token_info->token_chunks;
	while (list && list->cmds[0])
	{
		if (str_in_arr(list->cmds[0], BASH_CMDS))
		{
		}
		else if (access(get_path(list->cmds[0],
					token_info->env_arr), F_OK) != 0 \
					|| !strcmp(list->cmds[0], ""))
		{
			printf("bash: %s: command not found\n", list->start->word);
			token_info->cmd_start = list->next;
		}
		list = list->next;
	}
	return (token_info->cmd_start);
}

void print_cmd_list(t_cmd *cmd_list, int recur_depth)
{
    t_cmd *current = cmd_list;

    while (current)
    {
        // Print the current depth
        for (int i = 0; i < recur_depth; i++)
        {
            printf("  ");
        }

        // Print the type of command
        if (current->e_operator == CMD_LIST)
        {
            printf("CMD_LIST: (\n");
            print_cmd_list((t_cmd *)current->ptr, recur_depth + 1);

            for (int i = 0; i < recur_depth; i++)
            {
                printf("  ");
            }
            printf(")\n");
        }
        else if (current->e_operator == PIPE_LIST)
        {
			t_token *tok_head = (t_token *)current->ptr;
			printf("PIPE_LIST: %s: [", ((t_token *)current->ptr)->word);
			while(tok_head)
			{
				printf("'%s' ", tok_head->word);
				tok_head = tok_head->next;
			}
			printf("]\n");
        }

        // Move to the next command in the list
        current = current->next;
    }
}


int	main(int ac, char **av, char **env)
{
	void			(*ctrl_c_handler)(int);
	t_token_info	*token_info;
	char			**env_list;
	HIST_ENTRY		**history;
	char			*user_input;
	int	g_errno;

	ctrl_c_handler = signal(SIGINT, ctrl_c_function);
	signal(SIGQUIT, SIG_IGN);
	history = history_list();
	t_env *global_env = arr2env(env, env_list);

	while (1)
	{
		g_errno = 0;
		user_input = readline(SHELL_MSG);
		if (!user_input) //ctrl + D
			break ;
		if (ft_strlen(user_input) > 0)
		{
			add_history(user_input);
			token_info = process_input(user_input, global_env);
			token_info->env_arr = env;
			t_cmd *cmd_list= get_cmd_list(token_info);
			if (g_errno == 0)
				execution(token_info, cmd_list);
			cmd_list_free(&cmd_list);
			/*if (scan_cmd(token_info) && !token_info->has_error)
			{
				if (str_in_arr(token_info->cmd_start->cmds[0], BASH_CMDS))
					bash_cmd(env, token_info);
				else if (get_length_of_list(token_info->token_chunks) == 1)
					//run_cmds(env, token_info); //THIS IS TO BE REPLACED
			}*/
			print_cmd_list(cmd_list, 0);
			print_tokens(token_info, 'l');
			free_TokenList(token_info);
		}
	}
	exit(0);
}

