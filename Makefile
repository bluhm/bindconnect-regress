#	$OpenBSD$

PROG=		bindconnect
LDADD=		-lpthread
DPADD=		${LIBPTHREAD}
WARNINGS=	yes

CLEANFILES=	ktrace.out

${REGRESS_TARGETS}: ${PROG}

REGRESS_TARGETS +=	run-default
run-default:
	${SUDO} time ${KTRACE} ./${PROG}

REGRESS_TARGETS +=	run-bind
run-bind:
	${SUDO} time ${KTRACE} ./${PROG} -n 10 -s 2 -b 5 -c 1 -o 0

REGRESS_TARGETS +=	run-connect
run-connect:
	${SUDO} time ${KTRACE} ./${PROG} -n 10 -s 2 -b 0 -c 1 -o 5

REGRESS_TARGETS +=	run-bind-connect
run-bind-connect:
	${SUDO} time ${KTRACE} ./${PROG} -n 10 -s 2 -b 5 -c 1 -o 3

.include <bsd.regress.mk>
