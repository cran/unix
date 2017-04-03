#define R_NO_REMAP
#define STRICT_R_HEADERS

#include <Rinternals.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <unistd.h>
#include <signal.h>

extern void bail_if(int err, const char * what);

SEXP R_kill(SEXP pid, SEXP sig){
  bail_if(kill(Rf_asInteger(pid), Rf_asInteger(sig)) < 0, "send kill()");
  return R_NilValue;
}

SEXP R_getuid(){
  return Rf_ScalarInteger(getuid());
}

SEXP R_setuid(SEXP id){
  bail_if(setuid(Rf_asInteger(id)) < 0, "setuid()");
  return R_getuid();
}

SEXP R_getgid () {
  return Rf_ScalarInteger(getgid());
}

SEXP R_setgid(SEXP id){
  bail_if(setgid(Rf_asInteger(id)) < 0, "setuid()");
  return R_getgid();
}

SEXP R_getpid () {
  return Rf_ScalarInteger(getpid());
}

SEXP R_getppid () {
  return Rf_ScalarInteger(getppid());
}

SEXP R_getpgid () {
  return Rf_ScalarInteger(getpgid(0));
}

SEXP R_setpgid(SEXP pid){
  bail_if(setpgid(0, Rf_asInteger(pid)), "setpgid()");
  return R_getpgid();
}

SEXP R_getpriority () {
  return Rf_ScalarInteger(getpriority(PRIO_PROCESS, 0));
}

SEXP R_setpriority(SEXP prio){
  bail_if(setpriority(PRIO_PROCESS, 0, Rf_asInteger(prio)) < 0, "setpriority()");
  return R_getpriority();
}
