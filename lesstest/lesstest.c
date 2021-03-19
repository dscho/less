#include <sys/wait.h>
#include "lesstest.h"

extern TermInfo terminfo;

int verbose = 0;
int less_quit = 0;
int details = 0;
char* lt_screen = "./lt_screen";

static char* testfile = NULL;

int usage(void) {
	fprintf(stderr, "usage: lesstest -o file.lt [-w#] [-h#] [--] less.exe [flags] textfile\n");
	fprintf(stderr, "   or: lesstest -t file.lt less.exe\n");
	return 0;
}

void child_handler(int signum) {
	int status;
	pid_t child = wait(&status);
	if (verbose) fprintf(stderr, "child %d died, status 0x%x\n", child, status);
}

int setup(int argc, char* const* argv) {
	char* logfile = NULL;
	int ch;
	while ((ch = getopt(argc, argv, "do:s:t:v")) != -1) {
		switch (ch) {
		case 'd':
			details = 1;
			break;
		case 'o':
			logfile = optarg;
			break;
		case 's':
			lt_screen = optarg;
			break;
		case 't':
			testfile = optarg;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			return usage();
		}
	}
	if (logfile != NULL && !log_open(logfile)) {
		fprintf(stderr, "cannot create %s: %s\n", logfile, strerror(errno));
		return 0;
	}
	return 1;
}

int main(int argc, char* const* argv, char* const* envp) {
	signal(SIGCHLD, child_handler);
	if (!setup(argc, argv))
		return RUN_ERR;
	setup_term();
	int ok = 0;
	if (testfile != NULL) { // run existing test
		if (optind+1 != argc) {
			usage();
			return RUN_ERR;
		}
		ok = run_testfile(testfile, argv[optind]);
	} else { // gen; create new test
		if (optind+2 > argc) {
			usage();
			return RUN_ERR;
		}
		log_file_header();
		printf("%s%s", terminfo.init_term, terminfo.enter_keypad);
		ok = run_interactive(argv+optind, argc-optind, envp);
		printf("%s%s", terminfo.exit_keypad, terminfo.deinit_term);
		log_close();
	}
	return ok ? RUN_OK : RUN_ERR;
}
