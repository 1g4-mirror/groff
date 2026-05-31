// hdb.cpp
extern ELT *DBInit();
extern ELT *DBRead(FILE *);

// hgraph.cpp
extern void HGPrintElt(ELT *, int);

// hpoint.cpp
extern POINT *PTInit();
extern POINT *PTMakePoint(double, double, POINT **);

// main.cpp
extern void *grnmalloc(size_t, const char *);
extern void savebounds(double, double);
