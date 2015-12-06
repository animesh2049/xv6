8500 // Shell.
8501 
8502 #include "types.h"
8503 #include "user.h"
8504 #include "fcntl.h"
8505 
8506 // Parsed command representation
8507 #define EXEC  1
8508 #define REDIR 2
8509 #define PIPE  3
8510 #define LIST  4
8511 #define BACK  5
8512 
8513 #define MAXARGS 10
8514 
8515 struct cmd {
8516   int type;
8517 };
8518 
8519 struct execcmd {
8520   int type;
8521   char *argv[MAXARGS];
8522   char *eargv[MAXARGS];
8523 };
8524 
8525 struct redircmd {
8526   int type;
8527   struct cmd *cmd;
8528   char *file;
8529   char *efile;
8530   int mode;
8531   int fd;
8532 };
8533 
8534 struct pipecmd {
8535   int type;
8536   struct cmd *left;
8537   struct cmd *right;
8538 };
8539 
8540 struct listcmd {
8541   int type;
8542   struct cmd *left;
8543   struct cmd *right;
8544 };
8545 
8546 struct backcmd {
8547   int type;
8548   struct cmd *cmd;
8549 };
8550 int fork1(void);  // Fork but panics on failure.
8551 void panic(char*);
8552 struct cmd *parsecmd(char*);
8553 
8554 // Execute cmd.  Never returns.
8555 void
8556 runcmd(struct cmd *cmd)
8557 {
8558   int p[2];
8559   struct backcmd *bcmd;
8560   struct execcmd *ecmd;
8561   struct listcmd *lcmd;
8562   struct pipecmd *pcmd;
8563   struct redircmd *rcmd;
8564 
8565   if(cmd == 0)
8566     exit();
8567 
8568   switch(cmd->type){
8569   default:
8570     panic("runcmd");
8571 
8572   case EXEC:
8573     ecmd = (struct execcmd*)cmd;
8574     if(ecmd->argv[0] == 0)
8575       exit();
8576     exec(ecmd->argv[0], ecmd->argv);
8577     printf(2, "exec %s failed\n", ecmd->argv[0]);
8578     break;
8579 
8580   case REDIR:
8581     rcmd = (struct redircmd*)cmd;
8582     close(rcmd->fd);
8583     if(open(rcmd->file, rcmd->mode) < 0){
8584       printf(2, "open %s failed\n", rcmd->file);
8585       exit();
8586     }
8587     runcmd(rcmd->cmd);
8588     break;
8589 
8590   case LIST:
8591     lcmd = (struct listcmd*)cmd;
8592     if(fork1() == 0)
8593       runcmd(lcmd->left);
8594     wait();
8595     runcmd(lcmd->right);
8596     break;
8597 
8598 
8599 
8600   case PIPE:
8601     pcmd = (struct pipecmd*)cmd;
8602     if(pipe(p) < 0)
8603       panic("pipe");
8604     if(fork1() == 0){
8605       close(1);
8606       dup(p[1]);
8607       close(p[0]);
8608       close(p[1]);
8609       runcmd(pcmd->left);
8610     }
8611     if(fork1() == 0){
8612       close(0);
8613       dup(p[0]);
8614       close(p[0]);
8615       close(p[1]);
8616       runcmd(pcmd->right);
8617     }
8618     close(p[0]);
8619     close(p[1]);
8620     wait();
8621     wait();
8622     break;
8623 
8624   case BACK:
8625     bcmd = (struct backcmd*)cmd;
8626     if(fork1() == 0)
8627       runcmd(bcmd->cmd);
8628     break;
8629   }
8630   exit();
8631 }
8632 
8633 int
8634 getcmd(char *buf, int nbuf)
8635 {
8636   printf(2, "$ ");
8637   memset(buf, 0, nbuf);
8638   gets(buf, nbuf);
8639   if(buf[0] == 0) // EOF
8640     return -1;
8641   return 0;
8642 }
8643 
8644 
8645 
8646 
8647 
8648 
8649 
8650 int
8651 main(void)
8652 {
8653   static char buf[100];
8654   int fd;
8655 
8656   // Assumes three file descriptors open.
8657   while((fd = open("console", O_RDWR)) >= 0){
8658     if(fd >= 3){
8659       close(fd);
8660       break;
8661     }
8662   }
8663 
8664   // Read and run input commands.
8665   while(getcmd(buf, sizeof(buf)) >= 0){
8666     if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
8667       // Clumsy but will have to do for now.
8668       // Chdir has no effect on the parent if run in the child.
8669       buf[strlen(buf)-1] = 0;  // chop \n
8670       if(chdir(buf+3) < 0)
8671         printf(2, "cannot cd %s\n", buf+3);
8672       continue;
8673     }
8674     if(fork1() == 0)
8675       runcmd(parsecmd(buf));
8676     wait();
8677   }
8678   exit();
8679 }
8680 
8681 void
8682 panic(char *s)
8683 {
8684   printf(2, "%s\n", s);
8685   exit();
8686 }
8687 
8688 int
8689 fork1(void)
8690 {
8691   int pid;
8692 
8693   pid = fork();
8694   if(pid == -1)
8695     panic("fork");
8696   return pid;
8697 }
8698 
8699 
8700 // Constructors
8701 
8702 struct cmd*
8703 execcmd(void)
8704 {
8705   struct execcmd *cmd;
8706 
8707   cmd = malloc(sizeof(*cmd));
8708   memset(cmd, 0, sizeof(*cmd));
8709   cmd->type = EXEC;
8710   return (struct cmd*)cmd;
8711 }
8712 
8713 struct cmd*
8714 redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd)
8715 {
8716   struct redircmd *cmd;
8717 
8718   cmd = malloc(sizeof(*cmd));
8719   memset(cmd, 0, sizeof(*cmd));
8720   cmd->type = REDIR;
8721   cmd->cmd = subcmd;
8722   cmd->file = file;
8723   cmd->efile = efile;
8724   cmd->mode = mode;
8725   cmd->fd = fd;
8726   return (struct cmd*)cmd;
8727 }
8728 
8729 struct cmd*
8730 pipecmd(struct cmd *left, struct cmd *right)
8731 {
8732   struct pipecmd *cmd;
8733 
8734   cmd = malloc(sizeof(*cmd));
8735   memset(cmd, 0, sizeof(*cmd));
8736   cmd->type = PIPE;
8737   cmd->left = left;
8738   cmd->right = right;
8739   return (struct cmd*)cmd;
8740 }
8741 
8742 
8743 
8744 
8745 
8746 
8747 
8748 
8749 
8750 struct cmd*
8751 listcmd(struct cmd *left, struct cmd *right)
8752 {
8753   struct listcmd *cmd;
8754 
8755   cmd = malloc(sizeof(*cmd));
8756   memset(cmd, 0, sizeof(*cmd));
8757   cmd->type = LIST;
8758   cmd->left = left;
8759   cmd->right = right;
8760   return (struct cmd*)cmd;
8761 }
8762 
8763 struct cmd*
8764 backcmd(struct cmd *subcmd)
8765 {
8766   struct backcmd *cmd;
8767 
8768   cmd = malloc(sizeof(*cmd));
8769   memset(cmd, 0, sizeof(*cmd));
8770   cmd->type = BACK;
8771   cmd->cmd = subcmd;
8772   return (struct cmd*)cmd;
8773 }
8774 
8775 
8776 
8777 
8778 
8779 
8780 
8781 
8782 
8783 
8784 
8785 
8786 
8787 
8788 
8789 
8790 
8791 
8792 
8793 
8794 
8795 
8796 
8797 
8798 
8799 
8800 // Parsing
8801 
8802 char whitespace[] = " \t\r\n\v";
8803 char symbols[] = "<|>&;()";
8804 
8805 int
8806 gettoken(char **ps, char *es, char **q, char **eq)
8807 {
8808   char *s;
8809   int ret;
8810 
8811   s = *ps;
8812   while(s < es && strchr(whitespace, *s))
8813     s++;
8814   if(q)
8815     *q = s;
8816   ret = *s;
8817   switch(*s){
8818   case 0:
8819     break;
8820   case '|':
8821   case '(':
8822   case ')':
8823   case ';':
8824   case '&':
8825   case '<':
8826     s++;
8827     break;
8828   case '>':
8829     s++;
8830     if(*s == '>'){
8831       ret = '+';
8832       s++;
8833     }
8834     break;
8835   default:
8836     ret = 'a';
8837     while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
8838       s++;
8839     break;
8840   }
8841   if(eq)
8842     *eq = s;
8843 
8844   while(s < es && strchr(whitespace, *s))
8845     s++;
8846   *ps = s;
8847   return ret;
8848 }
8849 
8850 int
8851 peek(char **ps, char *es, char *toks)
8852 {
8853   char *s;
8854 
8855   s = *ps;
8856   while(s < es && strchr(whitespace, *s))
8857     s++;
8858   *ps = s;
8859   return *s && strchr(toks, *s);
8860 }
8861 
8862 struct cmd *parseline(char**, char*);
8863 struct cmd *parsepipe(char**, char*);
8864 struct cmd *parseexec(char**, char*);
8865 struct cmd *nulterminate(struct cmd*);
8866 
8867 struct cmd*
8868 parsecmd(char *s)
8869 {
8870   char *es;
8871   struct cmd *cmd;
8872 
8873   es = s + strlen(s);
8874   cmd = parseline(&s, es);
8875   peek(&s, es, "");
8876   if(s != es){
8877     printf(2, "leftovers: %s\n", s);
8878     panic("syntax");
8879   }
8880   nulterminate(cmd);
8881   return cmd;
8882 }
8883 
8884 struct cmd*
8885 parseline(char **ps, char *es)
8886 {
8887   struct cmd *cmd;
8888 
8889   cmd = parsepipe(ps, es);
8890   while(peek(ps, es, "&")){
8891     gettoken(ps, es, 0, 0);
8892     cmd = backcmd(cmd);
8893   }
8894   if(peek(ps, es, ";")){
8895     gettoken(ps, es, 0, 0);
8896     cmd = listcmd(cmd, parseline(ps, es));
8897   }
8898   return cmd;
8899 }
8900 struct cmd*
8901 parsepipe(char **ps, char *es)
8902 {
8903   struct cmd *cmd;
8904 
8905   cmd = parseexec(ps, es);
8906   if(peek(ps, es, "|")){
8907     gettoken(ps, es, 0, 0);
8908     cmd = pipecmd(cmd, parsepipe(ps, es));
8909   }
8910   return cmd;
8911 }
8912 
8913 struct cmd*
8914 parseredirs(struct cmd *cmd, char **ps, char *es)
8915 {
8916   int tok;
8917   char *q, *eq;
8918 
8919   while(peek(ps, es, "<>")){
8920     tok = gettoken(ps, es, 0, 0);
8921     if(gettoken(ps, es, &q, &eq) != 'a')
8922       panic("missing file for redirection");
8923     switch(tok){
8924     case '<':
8925       cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
8926       break;
8927     case '>':
8928       cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
8929       break;
8930     case '+':  // >>
8931       cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
8932       break;
8933     }
8934   }
8935   return cmd;
8936 }
8937 
8938 
8939 
8940 
8941 
8942 
8943 
8944 
8945 
8946 
8947 
8948 
8949 
8950 struct cmd*
8951 parseblock(char **ps, char *es)
8952 {
8953   struct cmd *cmd;
8954 
8955   if(!peek(ps, es, "("))
8956     panic("parseblock");
8957   gettoken(ps, es, 0, 0);
8958   cmd = parseline(ps, es);
8959   if(!peek(ps, es, ")"))
8960     panic("syntax - missing )");
8961   gettoken(ps, es, 0, 0);
8962   cmd = parseredirs(cmd, ps, es);
8963   return cmd;
8964 }
8965 
8966 struct cmd*
8967 parseexec(char **ps, char *es)
8968 {
8969   char *q, *eq;
8970   int tok, argc;
8971   struct execcmd *cmd;
8972   struct cmd *ret;
8973 
8974   if(peek(ps, es, "("))
8975     return parseblock(ps, es);
8976 
8977   ret = execcmd();
8978   cmd = (struct execcmd*)ret;
8979 
8980   argc = 0;
8981   ret = parseredirs(ret, ps, es);
8982   while(!peek(ps, es, "|)&;")){
8983     if((tok=gettoken(ps, es, &q, &eq)) == 0)
8984       break;
8985     if(tok != 'a')
8986       panic("syntax");
8987     cmd->argv[argc] = q;
8988     cmd->eargv[argc] = eq;
8989     argc++;
8990     if(argc >= MAXARGS)
8991       panic("too many args");
8992     ret = parseredirs(ret, ps, es);
8993   }
8994   cmd->argv[argc] = 0;
8995   cmd->eargv[argc] = 0;
8996   return ret;
8997 }
8998 
8999 
9000 // NUL-terminate all the counted strings.
9001 struct cmd*
9002 nulterminate(struct cmd *cmd)
9003 {
9004   int i;
9005   struct backcmd *bcmd;
9006   struct execcmd *ecmd;
9007   struct listcmd *lcmd;
9008   struct pipecmd *pcmd;
9009   struct redircmd *rcmd;
9010 
9011   if(cmd == 0)
9012     return 0;
9013 
9014   switch(cmd->type){
9015   case EXEC:
9016     ecmd = (struct execcmd*)cmd;
9017     for(i=0; ecmd->argv[i]; i++)
9018       *ecmd->eargv[i] = 0;
9019     break;
9020 
9021   case REDIR:
9022     rcmd = (struct redircmd*)cmd;
9023     nulterminate(rcmd->cmd);
9024     *rcmd->efile = 0;
9025     break;
9026 
9027   case PIPE:
9028     pcmd = (struct pipecmd*)cmd;
9029     nulterminate(pcmd->left);
9030     nulterminate(pcmd->right);
9031     break;
9032 
9033   case LIST:
9034     lcmd = (struct listcmd*)cmd;
9035     nulterminate(lcmd->left);
9036     nulterminate(lcmd->right);
9037     break;
9038 
9039   case BACK:
9040     bcmd = (struct backcmd*)cmd;
9041     nulterminate(bcmd->cmd);
9042     break;
9043   }
9044   return cmd;
9045 }
9046 
9047 
9048 
9049 
