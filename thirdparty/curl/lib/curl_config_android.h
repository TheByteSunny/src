#pragma once

#define PACKAGE "curl"
#define PACKAGE_BUGREPORT "a suitable curl mailing list: https://curl.se/mail/"
#define PACKAGE_NAME "curl"
#define PACKAGE_STRING "curl 7.79.0-DEV"
#define PACKAGE_TARNAME "curl"
#define PACKAGE_VERSION "7.79.0-DEV"
#define VERSION "7.79.0-DEV"

#define STDC_HEADERS 1
#define HAVE_STDLIB_H 1
#define HAVE_STDIO_H 1
#define HAVE_STRING_H 1
#define HAVE_STRINGS_H 1
#define HAVE_STDBOOL_H 1
#define HAVE_MEMORY_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_FCNTL_H 1
#define HAVE_TIME_H 1

#define HAVE_BOOL_T 1
#define HAVE_LONGLONG 1
#define SIZEOF_INT 4
#define SIZEOF_SHORT 2
#define SIZEOF_CURL_OFF_T 8
#define SIZEOF_TIME_T 8

#define ENABLE_IPV6 0
#define HAVE_GETADDRINFO 1
#define HAVE_FREEADDRINFO 1
#define HAVE_GETPEERNAME 1
#define HAVE_GETSOCKNAME 1
#define HAVE_STRUCT_TIMEVAL 1
#define HAVE_STRUCT_SOCKADDR_STORAGE 1

#define HAVE_ZLIB_H 1
#define HAVE_LIBZ 1

#if defined(__aarch64__)
	#define OS "aarch64-unknown-linux-android"
	#define SIZEOF_LONG 8
	#define SIZEOF_SIZE_T 8
#elif defined(__arm__)
	#define OS "arm-unknown-linux-androideabi"
	#define SIZEOF_LONG 4
	#define SIZEOF_SIZE_T 4
#elif defined(__x86_64__)
	#define OS "x86_64-unknown-linux-android"
	#define SIZEOF_LONG 8
	#define SIZEOF_SIZE_T 8
#elif defined(__i386__)
	#define OS "i686-unknown-linux-android"
	#define SIZEOF_LONG 4
	#define SIZEOF_SIZE_T 4
#else
	#define OS "unknown-linux-android"
	#define SIZEOF_LONG 4
	#define SIZEOF_SIZE_T 4
#endif

#define HAVE_UNISTD_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_SOCKET_H 1
#define HAVE_SYS_IOCTL_H 1
#define HAVE_NETINET_IN_H 1
#define HAVE_NETINET_TCP_H 1
#define HAVE_ARPA_INET_H 1
#define HAVE_NETDB_H 1
#define HAVE_PTHREAD_H 1
#define HAVE_POLL_H 1

#define HAVE_POLL 1
#define HAVE_SELECT 1
#define HAVE_SOCKET 1
#define HAVE_RECV 1
#define HAVE_SEND 1
#define HAVE_CLOCK_GETTIME_MONOTONIC 1

#define USE_THREADS_POSIX 1

#define USE_OPENSSL 1
#define HAVE_OPENSSL_CRYPTO_H 1
#define HAVE_OPENSSL_ERR_H 1
#define HAVE_OPENSSL_PEM_H 1
#define HAVE_OPENSSL_RSA_H 1
#define HAVE_OPENSSL_SSL_H 1
#define HAVE_OPENSSL_X509_H 1
#define HAVE_OPENSSL_RAND_H 1
#define HAVE_OPENSSL_PKCS12_H 1
#define OPENSSL_NO_ENGINE 1

#define RECV_TYPE_RETV ssize_t
#define RECV_TYPE_ARG1 int
#define RECV_TYPE_ARG2 void *
#define RECV_TYPE_ARG3 size_t
#define RECV_TYPE_ARG4 int

#define SEND_TYPE_RETV ssize_t
#define SEND_QUAL_ARG2 const
#define SEND_TYPE_ARG1 int
#define SEND_TYPE_ARG2 void *
#define SEND_TYPE_ARG3 size_t
#define SEND_TYPE_ARG4 int

#define HAVE_STRCASECMP 1
#define HAVE_STRNCASECMP 1

#define HAVE_ERRNO_H 1

#define SOCKERRNO (errno)
#define SET_SOCKERRNO(x) (errno = (x))

#define HAVE_FCNTL 1
#define HAVE_FCNTL_O_NONBLOCK 1
