#include <Rinternals.h>
#include <Rembedded.h>
#include <R_ext/Rdynload.h>
#include <string.h>

//Because mkString() segfaulst for NULL
#define make_string(x) x ? Rf_mkString(x) : ScalarString(NA_STRING)

//Define in exec.c
extern void bail_if(int err, const char * what);

//For: aa_change_profile()
#ifdef HAVE_APPARMOR
#include <sys/apparmor.h>
#endif

//For setuid(), rlimit, etc()
#include <unistd.h>
#include <sys/resource.h>

// Missing on Solaris
#ifndef RLIMIT_NPROC
#define RLIMIT_NPROC -1
#endif

#ifndef RLIMIT_MEMLOCK
#define RLIMIT_MEMLOCK -1
#endif

// Missing on OpenBSD
#ifndef RLIMIT_AS
#define RLIMIT_AS RLIMIT_DATA
#endif

// Order should match the R function
static int rlimit_types[9] = {
  RLIMIT_AS, //0
  RLIMIT_CORE, //1
  RLIMIT_CPU, //2
  RLIMIT_DATA, //3
  RLIMIT_FSIZE, //4
  RLIMIT_MEMLOCK, //5
  RLIMIT_NOFILE, //6
  RLIMIT_NPROC, //7
  RLIMIT_STACK, //8
};

SEXP R_safe_build(void){
#ifdef SYS_BUILD_SAFE
  return ScalarLogical(TRUE);
#else
  return ScalarLogical(FALSE);
#endif
}

SEXP R_have_apparmor(void){
#ifdef HAVE_APPARMOR
  return ScalarLogical(TRUE);
#else
  return ScalarLogical(FALSE);
#endif
}

SEXP R_set_tempdir(SEXP path){
#ifdef SYS_BUILD_SAFE
  const char * tmpdir = CHAR(STRING_ELT(path, 0));
  R_TempDir = strdup(tmpdir);
#else
  Rf_error("Cannot set tempdir(), sys has been built without SYS_BUILD_SAFE");
#endif
  return path;
}

SEXP R_set_interactive(SEXP set){
#ifdef SYS_BUILD_SAFE
  extern Rboolean R_Interactive;
  R_Interactive = asLogical(set);
#else
  Rf_error("Cannot set interactive(), sys has been built without SYS_BUILD_SAFE");
#endif
  return set;
}

//VECTOR of length n;
SEXP R_set_rlimits(SEXP limitvec){
  if(!Rf_isNumeric(limitvec))
    Rf_error("limitvec is not numeric");
  size_t len = sizeof(rlimit_types)/sizeof(rlimit_types[0]);
  if(Rf_length(limitvec) != len)
    Rf_error("limitvec wrong size");
  for(int i = 0; i < len; i++){
    int resource = rlimit_types[i];
    double val = REAL(limitvec)[i];
    if(resource < 0 || val == 0 || ISNA(val))
      continue;
    rlim_t rlim_val = R_finite(val) ? val : RLIM_INFINITY;
    //Rprintf("Setting %d to %d\n", resource,  rlim_val);
    struct rlimit lim = {rlim_val, rlim_val};
    bail_if(setrlimit(resource, &lim) < 0, "setrlimit()");
  }
  return R_NilValue;
}

/*** Only on Debian/Ubuntu systems ***/
SEXP R_aa_change_profile(SEXP profile){
#ifdef HAVE_APPARMOR
  const char * profstr = CHAR(STRING_ELT(profile, 0));
  bail_if(aa_change_profile (profstr) < 0, "aa_change_profile()");
#endif
  return R_NilValue;
}

SEXP R_aa_is_enabled(void){
#ifndef HAVE_APPARMOR
  return R_NilValue;
#else
  return ScalarLogical(aa_is_enabled());
#endif //HAVE_APPARMOR
}

SEXP R_aa_getcon(void){
#ifndef HAVE_APPARMOR
  return R_NilValue;
#else
  char * con = NULL;
  char * mode = NULL;
  if(!aa_getcon (&con, &mode))
    return R_NilValue;
  SEXP out = PROTECT(allocVector(VECSXP, 2));
  SET_VECTOR_ELT(out, 0, make_string(con));
  SET_VECTOR_ELT(out, 1, make_string(mode));
  UNPROTECT(1);
  return out;
#endif //HAVE_APPARMOR
}
