#ifndef CD_H
#define CD_H 

int cd(char **token_ptr, char **prompt_ptr, char **cwd_ptr, char **prwd_ptr, char *home_path);
void change_prompt(char **prompt_ptr, char *new_cwd);

#endif
