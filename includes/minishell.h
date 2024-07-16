#ifndef MINISHELL_H
# define MINISHELL_H

# include <stdio.h>
# include <readline/readline.h>
# include <readline/history.h>
# include "colors.h"
# include "libft.h"
# include <stdlib.h>
# include <signal.h>
# include <unistd.h>
# include <sys/wait.h>
# include <dirent.h>
# include <stdbool.h>
# include <stdarg.h>
# include <fcntl.h>

# define SHELL_MSG "minishell$ "
# define SPACE_CHAR " \f\v\t\r\n"
# define INVALID_ENV_CHAR "[]<>!@$%%^-+~/ "
# define BASH_CMDS "env,cd,unset,export,exit"
# define BRACE -125
# define INVALID_CHARS "{};*\\"
# define SHELL_OPERATORS "|><()&"

# define T_PIPE 'p'
# define T_CMD 'c'
# define T_REDIR 'r'

# define T_OR 'o'
# define T_AND 'a'
# define T_SUBSHELL '('

typedef struct s_env
{
	char			*name;
	char			*value;
	struct s_env	*next;
}	t_env;

typedef struct s_token
{
	char			*word;
	char			type;
	char			*infile; //token chunk only
	char			*outfile; //token chunk only
	char			outfile_mode; //token chunk only
	char			**cmds; //token chunk only
	struct s_token	*start; //token chunk only
	char			*heredoc; //token chunk only
	struct s_token	*next; //for BOTH
}	t_token;


typedef struct s_token_info
{
	t_token		*token_list;
	t_token		*quote_list_buffer;
	t_token		*token_chunks;
	int			size;
	char		*str;
	t_token		*cmd_start;
	char		**env_arr;
	bool		has_error;
	t_env		*global_env;
}	t_token_info;

extern int	g_errno; // Global errno is defined here

typedef struct s_exe
{
	t_list	*heredoc; //heredoc list that contains the piped fds
	int		**pipe_fd; //pipes for fds
	int		pipe_count; //number of pipes
	int		infile; //infile fd
	int		outfile; //outfile fd
	int		tmpstdin; //original stdin used to restore
	int		tmpstdout; //original stdout used to restore
	int		runtime_error; //check for errors during execution
	int		has_child; //check for child processes for non builtin funcs
}	t_exe;

typedef struct s_io
{
	enum
	{
		IO_AIN,
		IO_AOUT,
		IO_IN,
		IO_OUT
	}	e_type; //type of redirection, AIN<<, AOUT>>, IN<, OUT>
	t_list		*value; //filename or delimiter for heredoc
	struct s_io	*next; // pointer to next node in io_list
}	t_io;

typedef struct s_pipe
{
	t_list			*argv; //list of arguments
	t_io			*io_list; //io_list for the pipe
	struct s_pipe	*next; //pointer to next pipe in pipe_list
}	t_pipe;

typedef struct s_error
{
	char	error_type;
	char	*subject;
}	t_error;

typedef struct s_cmd
{
	enum
	{
		OP_START,
		OP_AND,
		OP_OR
	}	e_operator; //operator for cmd list when initialised
	enum
	{
		PIPE_LIST,
		CMD_LIST
	}	e_type; 
	//CMD_LIST for grouped commands in subshells
	//PIPE_LIST for simple commands with pipes
	void			*ptr; //ptr to the struct it is holding
	struct s_cmd	*next; //pointer to next node in cmd_list
}	t_cmd;

// utils
int				get_length_of_list(t_token *head);
int				newline(int var);

// debugging
int				print_tokens(t_token_info *token_list, char format);

// free utils
int				free_TokenList(t_token_info *token_list);

// tokenization
t_token			*scan_cmd(t_token_info *token_list);
t_token_info	*process_input(char *str, t_env *env_arr);
t_token			*tokenize(char *string, t_token_info *token_list);
char			**tokens2arr(t_token *token_chunk, t_token *str_end);
t_token			*append_tok(t_token *token, t_token **head);
t_token			*tok(char *word, char type);
int				is_token_valid(char *str, t_token_info *token_info);

// environment variables
t_env			*arr2env(char **env, char **env_arr);
t_env			*get_env_var(char *var_name, t_env **env);
int				print_env(t_env **env_list, char mode);
char			**env2arr(t_env *env);
char			*expand_env(char *string, t_token_info *token_info);
t_env			*new_env(char *name, char *value);
int				unset_env(char *var_name, t_env **envList, 
					t_token_info *token_list);
t_env			*append_env(t_env *env, t_env **envList, char **env_arr);
int				export(t_token_info *token_info);

// quotes
char			toggle_quote_state(char quote, char c);
char			*split_into_quotes(char *str, t_token *tokens, t_token_info *token_info);
int				count_outermost_quotes(char *str);
bool			quote_alternate(char c, char *quote);

//chunking
void			chunk_tokens(t_token_info *token_list);
int				handle_redir(t_token *head, t_token *token_chunk);

// to be removed
void			run_cmds(char **env, t_token_info *token_info);
char			*get_path(char *cmd, char **env);

// cmd parse
t_cmd	*get_cmd_list(t_token_info *token_info);
t_cmd	*parse_cmd_list(t_token *token_list);
void	parse_cmd_recurse(t_token *token_list, t_cmd **buffer);
void	parse_cmd_next(t_token *token_list, t_cmd **buffer);
void  print_syntax_error(t_token *token);
int print_error(t_error error, t_token_info *token_info);
t_cmd	*cmd_list_init(int operator);
t_pipe	*pipe_list_init(void);
int	parse_pipe_next(t_pipe **buffer);
void	pipe_new_arg(t_token *token_list, t_pipe *buffer);
t_pipe	*parse_pipe_list_sub(t_token *token_list);
int	parse_pipe_list(t_token *token_list, t_cmd *buffer);
int	parse_io_list(t_token *token_list, t_io **io_list);
int	is_io_token(t_token *token);
int check_io_type(t_token *token);
t_io	*io_list_init(int type);
int check_valid_filename(t_token *token);
void	cmd_list_free(t_cmd **cmd_list);
void	pipe_list_free(t_pipe **pipe_list);
void	free_args(void *content);
void	io_list_free(t_io **io_list);
int		is_pipe_token(t_token *token);
int		is_spec_token(t_token *token);

// execution
void 	execution(t_token_info *token_info, t_cmd *cmd_list);
void	executor_cmd_list(t_token_info *token_info, t_exe *exec, t_cmd *cmd);
void	executor_pipe_list(t_token_info *token_info, t_exe *exec, t_pipe *pipe);
void	executor(t_token_info *token_info, t_exe *exec, t_pipe *p);
t_exe	*executor_init(void);
void	executor_free(t_exe **exec);
void	executor_init_pipefd(t_exe *exec, t_pipe *pipe);
void	executor_wait_pipe(void);
void	executor_free_pipefd(t_exe *exec);
void	exec_redir_set(t_exe *exec, t_pipe *p);
void	exec_redir_reset(t_exe *exec);
void	*ft_list_to_array(t_list *lst, int size);
int	executor_ambiguous(t_exe *exec, t_list *value, char *env_val);
void	executor_io_in(t_exe *exec, t_io *io);
void	executor_io_out(t_exe *exec, t_io *io);
int	executor_check_file_error(t_exe *exec, char *filename);
void	executor_io_list(t_token_info *token_info, t_exe *exec, t_io *io);
int	ft_cd(t_token_info *token_info, char **args);
int	ft_pwd(t_token_info *token_info, char **args);
int	ft_echo(t_token_info *token_info, char **args);
int	ft_unset(t_token_info *token_info, char **args);
int	ft_env(t_token_info *token_info, char **args);
int	ft_export(t_token_info *token_info, char **args);
int	ft_exit(t_token_info *token_info, char **args);
int	get_flag(char **args);
void	free_env_node(t_env *env);
int	check_valid_identifier(char *arg, char *str, char *type);
void	find_and_delete(t_token_info *token_info, char *key);
void	heredoc_cmd_queue(t_exe *exec, t_cmd *cmd_list,
			int state, t_token_info *token_info);
void	heredoc_pipe_queue(t_exe *exec, t_pipe *pipe_list, int state,
			t_token_info *token_info);
void	heredoc_io_list_enqueue(t_exe *exec, t_io *io_list, t_token_info *token_info);
void	heredoc_io_list_dequeue(t_exe *exec, t_io *io_list);
void	heredoc_cmd_queue(t_exe *exec, t_cmd *cmd_list,
			int state, t_token_info *token_info);
void	heredoc_pipe_queue(t_exe *exec, t_pipe *pipe_list, int state,
			t_token_info *token_info);
void	heredoc_io_list_enqueue(t_exe *exec, t_io *io_list, t_token_info *token_info);
void	heredoc_io_list_dequeue(t_exe *exec, t_io *io_list);
static char	*comb_cmd(char **paths, char *rawcmd);
int	get_abspath(char **env, char **cmd);
static void	child_close_fd(t_exe *exec, t_pipe *p);
void	executor_non_bi(t_token_info *token_info, t_exe *exec,
			t_pipe *p, char **argv);
void	free_doublearray(char **split);
void	executor_bi(t_token_info *token_info, char **command, int bif_n);
int	exec_is_builtin(char *command);


# define NOT_WITHIN_QUOTE '\0'


#endif
