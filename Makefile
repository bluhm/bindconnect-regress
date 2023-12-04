#	$OpenBSD$

PROG=		bindstress
LDADD=		-lpthread
DPADD=		${LIBPTHREAD}
WARNINGS=	yes

CLEANFILES=	ktrace.out

run-regress-bindstress:
	${SUDO} time ${KTRACE} ./bindstress

.include <bsd.regress.mk>
