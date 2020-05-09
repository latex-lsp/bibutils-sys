/*
 * bibcore.c
 *
 * Copyright (c) Chris Putnam 2005-2020
 *
 * Source code released under the GPL version 2
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "bibutils.h"

/* internal includes */
#include "reftypes.h"
#include "charsets.h"
#include "str_conv.h"
#include "is_ws.h"

/* illegal modes to pass in, but use internally for consistency */
#define BIBL_INTERNALIN   (BIBL_LASTIN+1)
#define BIBL_INTERNALOUT  (BIBL_LASTOUT+1)

#define debug_set( p ) ( (p)->verbose > 1 )
#define verbose_set( p ) ( (p)->verbose )

static void
report_params( FILE *fp, const char *f, param *p )
{
	fflush( NULL );

	fprintf( fp, "-------------------params start for %s\n", f );
	fprintf( fp, "\tprogname='%s'\n\n", p->progname );

	fprintf( fp, "\treadformat=%d", p->readformat );
	switch ( p->readformat ) {
		case BIBL_INTERNALIN:   fprintf( fp, " (BIBL_INTERNALIN)\n" );   break;
		case BIBL_MODSIN:       fprintf( fp, " (BIBL_MODSIN)\n" );       break;
		case BIBL_BIBTEXIN:     fprintf( fp, " (BIBL_BIBTEXIN)\n" );     break;
		case BIBL_RISIN:        fprintf( fp, " (BIBL_RISIN)\n" );        break;
		case BIBL_ENDNOTEIN:    fprintf( fp, " (BIBL_ENDNOTEIN)\n" );    break;
		case BIBL_COPACIN:      fprintf( fp, " (BIBL_COPACIN)\n" );      break;
		case BIBL_ISIIN:        fprintf( fp, " (BIBL_ISIIN)\n" );        break;
		case BIBL_MEDLINEIN:    fprintf( fp, " (BIBL_MEDLINEIN)\n" );    break;
		case BIBL_ENDNOTEXMLIN: fprintf( fp, " (BIBL_ENDNOTEXMLIN)\n" ); break;
		case BIBL_BIBLATEXIN:   fprintf( fp, " (BIBL_BIBLATEXIN)\n" );   break;
		case BIBL_EBIIN:        fprintf( fp, " (BIBL_EBIIN)\n" );        break;
		case BIBL_WORDIN:       fprintf( fp, " (BIBL_WORDIN)\n" );       break;
		case BIBL_NBIBIN:       fprintf( fp, " (BIBL_NBIBIN)\n" );       break;
		default:                fprintf( fp, " (Illegal value)\n" );     break;
	}
	fprintf( fp, "\tcharsetin=%d\n", p->charsetin );
	fprintf( fp, "\tcharsetin_src=%d", p->charsetin_src );
	switch ( p->charsetin_src ) {
		case BIBL_SRC_DEFAULT:  fprintf( fp, " (BIBL_SRC_DEFAULT)\n" ); break;
		case BIBL_SRC_FILE:     fprintf( fp, " (BIBL_SRC_FILE)\n" );    break;
		case BIBL_SRC_USER:     fprintf( fp, " (BIBL_SRC_USER)\n" );    break;
		default:                fprintf( fp, " (Illegal value)\n" );    break;
	}
	fprintf( fp, "\tutf8in=%d\n", p->utf8in );
	fprintf( fp, "\tlatexin=%d\n", p->latexin );
	fprintf( fp, "\txmlin=%d\n\n", p->xmlin );

	fprintf( fp, "\twriteformat=%d", p->writeformat );
	switch ( p->writeformat ) {
		case BIBL_INTERNALOUT:  fprintf( fp, " (BIBL_INTERNALOUT)\n" );  break;
		case BIBL_ADSABSOUT:    fprintf( fp, " (BIBL_ADSABSOUT)\n" );    break;
		case BIBL_BIBTEXOUT:    fprintf( fp, " (BIBL_BIBTEXOUT)\n" );    break;
		case BIBL_ENDNOTEOUT:   fprintf( fp, " (BIBL_ENDNOTEOUT)\n" );   break;
		case BIBL_ISIOUT:       fprintf( fp, " (BIBL_ISIOUT)\n" );       break;
		case BIBL_MODSOUT:      fprintf( fp, " (BIBL_MODSOUT)\n" );      break;
		case BIBL_NBIBOUT:      fprintf( fp, " (BIBL_NBIBOUT)\n" );      break;
		case BIBL_RISOUT:       fprintf( fp, " (BIBL_RISOUT)\n" );       break;
		case BIBL_WORD2007OUT:  fprintf( fp, " (BIBL_WORD2007OUT)\n" );  break;
		default:                fprintf( fp, " (Illegal value)\n");      break;
	}
	fprintf( fp, "\tcharsetout=%d\n", p->charsetout );
	fprintf( fp, "\tcharsetout_src=%d", p->charsetout_src );
	switch ( p->charsetout_src ) {
		case BIBL_SRC_DEFAULT:  fprintf( fp, " (BIBL_SRC_DEFAULT)\n" ); break;
		case BIBL_SRC_FILE:     fprintf( fp, " (BIBL_SRC_FILE)\n" );    break;
		case BIBL_SRC_USER:     fprintf( fp, " (BIBL_SRC_USER)\n" );    break;
		default:                fprintf( fp, " (Illegal value)\n" );    break;
	}
	fprintf( fp, "\tutf8out=%d\n", p->utf8out );
	fprintf( fp, "\tutf8bom=%d\n", p->utf8bom );
	fprintf( fp, "\tlatexout=%d\n", p->latexout );
	fprintf( fp, "\txmlout=%d\n", p->xmlout );
	fprintf( fp, "-------------------params end for %s\n", f );

	fflush( fp );
}

/* bibl_duplicateparams()
 *
 * Returns status of BIBL_OK or BIBL_ERR_MEMERR
 */
static int
bibl_duplicateparams( param *np, param *op )
{
	int status;

	slist_init( &(np->asis) );
	status = slist_copy( &(np->asis), &(op->asis ) );
	if ( status!=SLIST_OK ) return BIBL_ERR_MEMERR;

	slist_init( &(np->corps) );
	status = slist_copy( &(np->corps), &(op->corps ) );
	if ( status!=SLIST_OK ) return BIBL_ERR_MEMERR;

	if ( !op->progname ) np->progname = NULL;
	else {
		np->progname = strdup( op->progname );
		if ( !np->progname ) return BIBL_ERR_MEMERR;
	}

	np->readformat    = op->readformat;
	np->charsetin     = op->charsetin;
	np->charsetin_src = op->charsetin_src;
	np->utf8in        = op->utf8in;
	np->latexin       = op->latexin;
	np->xmlin         = op->xmlin;

	np->writeformat    = op->writeformat;
	np->charsetout     = op->charsetout;
	np->charsetout_src = op->charsetout_src;
	np->utf8out        = op->utf8out;
	np->utf8bom        = op->utf8bom;
	np->latexout       = op->latexout;
	np->xmlout         = op->xmlout;
	np->nosplittitle   = op->nosplittitle;

	np->verbose          = op->verbose;
	np->format_opts      = op->format_opts;
	np->addcount         = op->addcount;
	np->output_raw       = op->output_raw;
	np->singlerefperfile = op->singlerefperfile;

	np->readf     = op->readf;
	np->processf  = op->processf;
	np->cleanf    = op->cleanf;
	np->typef     = op->typef;
	np->convertf  = op->convertf;
	np->headerf   = op->headerf;
	np->footerf   = op->footerf;
	np->assemblef = op->assemblef;
	np->writef    = op->writef;

	np->all       = op->all;
	np->nall      = op->nall;

	return BIBL_OK;
}

/* bibl_setreadparams()
 *
 * Returns status of BIBL_OK or BIBL_ERR_MEMERR
 */
static int
bibl_setreadparams( param *np, param *op )
{
	int status;
	status = bibl_duplicateparams( np, op );
	if ( status == BIBL_OK ) {
		np->utf8out        = 1;
		np->charsetout     = BIBL_CHARSET_UNICODE;
		np->charsetout_src = BIBL_SRC_DEFAULT;
		np->xmlout         = BIBL_XMLOUT_FALSE;
		np->latexout       = 0;
		np->writeformat    = BIBL_INTERNALOUT;
	}
	return status;
}

/* bibl_setwriteparams()
 *
 * Returns status of BIBL_OK or BIBL_ERR_MEMERR
 */
static int
bibl_setwriteparams( param *np, param *op )
{
	int status;
	status = bibl_duplicateparams( np, op );
	if ( status == BIBL_OK ) {
		np->xmlin         = 0;
		np->latexin       = 0;
		np->utf8in        = 1;
		np->charsetin     = BIBL_CHARSET_UNICODE;
		np->charsetin_src = BIBL_SRC_DEFAULT;
		np->readformat    = BIBL_INTERNALIN;
	}
	return status;
}

void
bibl_freeparams( param *p )
{
	if ( p ) {
		slist_free( &(p->asis) );
		slist_free( &(p->corps) );
		if ( p->progname ) free( p->progname );
	}
}

int
bibl_readasis( param *p, char *f )
{
	int status;

	if ( !p ) return BIBL_ERR_BADINPUT;
	if ( !f ) return BIBL_ERR_BADINPUT;

	status = slist_fill( &(p->asis), f, 1 );

	if ( status == SLIST_ERR_CANTOPEN ) return BIBL_ERR_CANTOPEN;
	else if ( status == SLIST_ERR_MEMERR ) return BIBL_ERR_MEMERR;
	return BIBL_OK;
}

int
bibl_readcorps( param *p, char *f )
{
	int status;

	if ( !p ) return BIBL_ERR_BADINPUT;
	if ( !f ) return BIBL_ERR_BADINPUT;

	status = slist_fill( &(p->corps), f, 1 );

	if ( status == SLIST_ERR_CANTOPEN ) return BIBL_ERR_CANTOPEN;
	else if ( status == 0 ) return BIBL_ERR_MEMERR;
	return BIBL_OK;
}

/* bibl_addtoasis()
 *
 * Returns BIBL_OK or BIBL_ERR_MEMERR
 */
int
bibl_addtoasis( param *p, char *d )
{
	int status;

	if ( !p ) return BIBL_ERR_BADINPUT;
	if ( !d ) return BIBL_ERR_BADINPUT;

	status = slist_addc( &(p->asis), d );

	return ( status==SLIST_OK )? BIBL_OK : BIBL_ERR_MEMERR;
}

/* bibl_addtocorps()
 *
 * Returns BIBL_OK or BIBL_ERR_MEMERR
 */
int
bibl_addtocorps( param *p, char *d )
{
	int status;

	if ( !p ) return BIBL_ERR_BADINPUT;
	if ( !d ) return BIBL_ERR_BADINPUT;

	status = slist_addc( &(p->corps), d );

	return ( status==SLIST_OK )? BIBL_OK : BIBL_ERR_MEMERR;
}

void
bibl_reporterr( int err )
{
	// Patch: Disable output logging
}

static int
bibl_illegalinmode( int mode )
{
	if ( mode < BIBL_FIRSTIN || mode > BIBL_LASTIN ) return 1;
	else return 0;
}

static int
bibl_illegaloutmode( int mode )
{
	if ( mode < BIBL_FIRSTOUT || mode > BIBL_LASTOUT ) return 1;
	else return 0;
}

static void
bibl_verbose_reference( fields *f, char *filename, long refnum )
{
	// Patch: Disable output logging
}

static void
bibl_verbose( bibl *bin, const char *msg1, const char *msg2 )
{
	// Patch: Disable output logging
}


/* extract_tag_value
 *
 * Extract the tag and the value for ALWAYS/DEFAULT
 * entries like: "GENRE:BIBUTILS|Masters thesis"
 *
 * tag = "GENRE:BIBUTILS"
 * value = "Masters thesis"
 */
static int
extract_tag_value( str *tag, str *value, char *p )
{
	str_empty( tag );
	while ( p && *p && *p!='|' ) {
		str_addchar( tag, *p );
		p++;
	}
	if ( str_memerr( tag ) ) return BIBL_ERR_MEMERR;

	if ( p && *p=='|' ) p++;

	str_empty( value );
	while ( p && *p ) {
		str_addchar( value, *p );
		p++;
	}
	if ( str_memerr( tag ) ) return BIBL_ERR_MEMERR;

	return BIBL_OK;
}

/* process_defaultadd()
 *
 * Add tag/value pairs that have "DEFAULT" processing
 * unless a tag/value pair with the same tag has already
 * been adding during reference processing.
 */
static int
process_defaultadd( fields *f, int reftype, param *r )
{
	int i, n, process, level, status, ret = BIBL_OK;
	str tag, value;
	char *p;

	strs_init( &tag, &value, NULL );

	for ( i=0; i<r->all[reftype].ntags; ++i ) {

		process = ((r->all[reftype]).tags[i]).processingtype;
		if ( process!=DEFAULT ) continue;

		level   = ((r->all[reftype]).tags[i]).level;
		p       = ((r->all[reftype]).tags[i]).newstr;

		status = extract_tag_value( &tag, &value, p );
		if ( status!=BIBL_OK ) {
			ret = status;
			goto out;
		}

		n = fields_find( f, tag.data, level );
		if ( n==FIELDS_NOTFOUND ) {
			status = fields_add( f, tag.data, value.data, level );
			if ( status!=FIELDS_OK ) {
				ret = BIBL_ERR_MEMERR;
				goto out;
			}
		}

	}
out:
	strs_free( &tag, &value, NULL );

	return ret;
}

/* process_alwaysadd()
 *
 * Add tag/value pair to reference from the ALWAYS 
 * processing type without exception (the difference from
 * DEFAULT processing).
 */
static int
process_alwaysadd( fields *f, int reftype, param *r )
{
	int i, process, level, status, ret = BIBL_OK;
	str tag, value;
	char *p;

	strs_init( &tag, &value, NULL );

	for ( i=0; i<r->all[reftype].ntags; ++i ) {

		process = ((r->all[reftype]).tags[i]).processingtype;
		if ( process!=ALWAYS ) continue;

		level   = ((r->all[reftype]).tags[i]).level;
		p       = ((r->all[reftype]).tags[i]).newstr;

		status = extract_tag_value( &tag, &value, p );
		if ( status!=BIBL_OK ) {
			ret = status;
			goto out;
		}

		status = fields_add( f, tag.data, value.data, level );
		if ( status!=FIELDS_OK ) {
			ret = BIBL_ERR_MEMERR;
			goto out;
		}
	}

out:
	strs_free( &tag, &value, NULL );

	return ret;
}

static int
read_refs( FILE *fp, bibl *bin, char *filename, param *p )
{
	int refnum = 0, bufpos = 0, ret=BIBL_OK, fcharset;/* = CHARSET_UNKNOWN;*/
	str reference, line;
	char buf[256]="";
	fields *ref;

	str_init( &reference );
	str_init( &line );
	while ( p->readf( fp, buf, sizeof(buf), &bufpos, &line, &reference, &fcharset ) ) {
		if ( reference.len==0 ) continue;
		ref = fields_new();
		if ( !ref ) {
			ret = BIBL_ERR_MEMERR;
			bibl_free( bin );
			goto out;
		}
		if ( p->processf( ref, reference.data, filename, refnum+1, p )){
			ret = bibl_addref( bin, ref );
			if ( ret!=BIBL_OK ) {
				bibl_free( bin );
				fields_delete( ref );
				goto out;
			}
			refnum += 1;
		} else {
			fields_delete( ref );
		}
		str_empty( &reference );
		if ( fcharset!=CHARSET_UNKNOWN ) {
			/* charset from file takes priority over default, but
			 * not user-specified */
			if ( p->charsetin_src!=BIBL_SRC_USER ) {
				p->charsetin_src = BIBL_SRC_FILE;
				p->charsetin = fcharset;
				if ( fcharset!=CHARSET_UNICODE ) p->utf8in = 0;
			}
		}
	}
	if ( p->charsetin==CHARSET_UNICODE ) p->utf8in = 1;
out:
	str_free( &line );
	str_free( &reference );
	return ret;
}

/* Don't manipulate latex for URL's and the like */
static int
bibl_notexify( char *tag )
{
	char *protected[] = { "DOI", "URL", "REFNUM", "FILEATTACH", "FILE" };
	int i, nprotected = sizeof( protected ) / sizeof( protected[0] );
	for ( i=0; i<nprotected; ++i )
		if ( !strcasecmp( tag, protected[i] ) ) return 1;
	return 0;
}

/* bibl_fixcharsetdata()
 *
 * returns BIBL_OK or BIBL_ERR_MEMERR
 */
static int
bibl_fixcharsetdata( fields *ref, param *p )
{
	str *data;
	char *tag;
	long i, n;
	int ok;

	n = fields_num( ref );

	for ( i=0; i<n; ++i ) {

		tag  = fields_tag( ref, i, FIELDS_CHRP_NOUSE );
		data = fields_value( ref, i, FIELDS_STRP_NOUSE );

		if ( bibl_notexify( tag ) ) {
			ok = str_convert( data,
				p->charsetin,  0, p->utf8in,  p->xmlin,
				p->charsetout, 0, p->utf8out, p->xmlout );
		} else {
			ok = str_convert( data,
				p->charsetin,  p->latexin,  p->utf8in,  p->xmlin,
				p->charsetout, p->latexout, p->utf8out, p->xmlout );
		}

		if ( !ok ) return BIBL_ERR_MEMERR;
	}

	return BIBL_OK;
}

/* bibl_fixcharsets()
 *
 * returns BIBL_OK or BIBL_ERR_MEMERR
 */
static int
bibl_fixcharsets( bibl *b, param *p )
{
	int status;
	long i;

	for ( i=0; i<b->n; ++i ) {
		status = bibl_fixcharsetdata( b->ref[i], p );
		if ( status!=BIBL_OK ) return status;
	}

	return BIBL_OK;
}

static int
bibl_addcount( bibl *b )
{
	char buf[512];
	fields *ref;
	long i;
	int n;

	for ( i=0; i<b->n; ++i ) {

		ref = b->ref[i];

		n = fields_find( ref, "REFNUM", LEVEL_MAIN );
		if ( n==FIELDS_NOTFOUND ) continue;

		sprintf( buf, "_%ld", i+1 );
		str_strcatc( fields_value( ref, n, FIELDS_STRP_NOUSE ), buf );
		if ( str_memerr( fields_value( ref, n, FIELDS_STRP_NOUSE ) ) ) {
			return BIBL_ERR_MEMERR;
		}

	}

	return BIBL_OK;
}

static int
generate_citekey( fields *f, long nref )
{
	int n1, n2, status, ret;
	char *p, buf[100];
	str citekey;

	str_init( &citekey );

	n1 = fields_find( f, "AUTHOR", LEVEL_MAIN );
	if ( n1==FIELDS_NOTFOUND ) n1 = fields_find( f, "AUTHOR:ASIS", LEVEL_MAIN );
	if ( n1==FIELDS_NOTFOUND ) n1 = fields_find( f, "AUTHOR:CORP", LEVEL_MAIN );
	if ( n1==FIELDS_NOTFOUND ) n1 = fields_find( f, "AUTHOR", LEVEL_ANY );
	if ( n1==FIELDS_NOTFOUND ) n1 = fields_find( f, "AUTHOR:ASIS", LEVEL_ANY );
	if ( n1==FIELDS_NOTFOUND ) n1 = fields_find( f, "AUTHOR:CORP", LEVEL_ANY );

	n2 = fields_find( f, "DATE:YEAR", LEVEL_MAIN );
	if ( n2==FIELDS_NOTFOUND ) n2 = fields_find( f, "DATE:YEAR", LEVEL_ANY );
	if ( n2==FIELDS_NOTFOUND ) n2 = fields_find( f, "PARTDATE:YEAR", LEVEL_MAIN );
	if ( n2==FIELDS_NOTFOUND ) n2 = fields_find( f, "PARTDATE:YEAR", LEVEL_ANY );

	if ( n1!=FIELDS_NOTFOUND && n2!=FIELDS_NOTFOUND ) {

		p = fields_value( f, n1, FIELDS_CHRP_NOUSE );
		while ( p && *p && *p!='|' ) {
			if ( !is_ws( *p ) ) str_addchar( &citekey, *p ); 
			p++;
		}

		p = fields_value( f, n2, FIELDS_CHRP_NOUSE );
		while ( p && *p ) {
			if ( !is_ws( *p ) ) str_addchar( &citekey, *p );
			p++;
		}

	}

	else {
		sprintf( buf, "ref%ld", nref );
		str_strcpyc( &citekey, buf );
	}

	if ( str_memerr( &citekey ) ) {
		ret = -1;
		goto out;
	}

	status = fields_add( f, "REFNUM", str_cstr( &citekey ), LEVEL_MAIN );
	if ( status!=FIELDS_OK ) {
		ret = -1;
		goto out;
	}

	ret = fields_find( f, "REFNUM", LEVEL_MAIN );
out:
	str_free( &citekey );
	return ret;
}

static int
get_citekeys( bibl *bin, slist *citekeys )
{
	int n, status;
	fields *f;
	long i;

	for ( i=0; i<bin->n; ++i ) {
		f = bin->ref[i];
		n = fields_find( f, "REFNUM", LEVEL_ANY );
		if ( n==FIELDS_NOTFOUND ) n = generate_citekey( f, i+1 );
		if ( n!=FIELDS_NOTFOUND && fields_has_value( f, n ) ) {
			status = slist_add( citekeys, fields_value( f, n, FIELDS_STRP_NOUSE ) );
			if ( status!=SLIST_OK ) return BIBL_ERR_MEMERR;
		} else {
			status = slist_addc( citekeys, "" );
			if ( status!=SLIST_OK ) return BIBL_ERR_MEMERR;
		}
	}

	return BIBL_OK;
}

static int
identify_duplicates( bibl *b, slist *citekeys, int *dup )
{
	int i, j, ndup = 0;

	for ( i=0; i<citekeys->n-1; ++i ) {
		if ( dup[i]!=-1 ) continue;
		for ( j=i+1; j<citekeys->n; ++j ) {
			if ( !strcmp( slist_cstr( citekeys, i ),
			              slist_cstr( citekeys, j ) ) ) {
					dup[i] = i;
					dup[j] = i;
					ndup++;
			}
		}
	}

	return ndup;
}

static int
build_new_citekey( int nsame, str *old_citekey, str *new_citekey )
{
	const char abc[]="abcdefghijklmnopqrstuvwxyz";

	str_strcpy( new_citekey, old_citekey );

	while ( nsame >= 26 ) {
		str_addchar( new_citekey, 'a' );
		nsame -= 26;
	}

	if ( nsame>=0 ) str_addchar( new_citekey, abc[nsame] );

	return ( str_memerr( new_citekey ) ) ? BIBL_ERR_MEMERR : BIBL_OK;
}

static int
resolve_duplicates( bibl *b, slist *citekeys, int *dup )
{
	int nsame, n, i, j, status = BIBL_OK;
	str new_citekey, *ref_citekey;

	str_init( &new_citekey );

	for ( i=0; i<citekeys->n; ++i ) {

		if ( dup[i]==-1 ) continue;

		nsame = 0;

		for ( j=i; j<citekeys->n; ++j ) {

			if ( dup[j]!=i ) continue;

			dup[j] = -1;

			status = build_new_citekey( nsame, slist_str( citekeys, j ), &new_citekey );
			if ( status!=BIBL_OK ) goto out;

			n = fields_find( b->ref[j], "REFNUM", LEVEL_ANY );
			if ( n==FIELDS_NOTFOUND ) continue;

			ref_citekey = fields_value( b->ref[j], n, FIELDS_STRP_NOUSE );

			str_strcpy( ref_citekey, &new_citekey );
			if ( str_memerr( ref_citekey ) ) { status = BIBL_ERR_MEMERR; goto out; }

			nsame++;
		}
	}
out:
	str_free( &new_citekey );
	return status;
}

static int
identify_and_resolve_duplicate_citekeys( bibl *b, slist *citekeys )
{
	int i, *dup, ndup, status=BIBL_OK;

	dup = ( int * ) malloc( sizeof( int ) * citekeys->n );
	if ( !dup ) return BIBL_ERR_MEMERR;
	for ( i=0; i<citekeys->n; ++i ) dup[i] = -1;

	ndup = identify_duplicates( b, citekeys, dup );

	if ( ndup ) status = resolve_duplicates( b, citekeys, dup );

	free( dup );
	return status;
}

static int
uniqueify_citekeys( bibl *bin )
{
	slist citekeys;
	int status;

	slist_init( &citekeys );

	status = get_citekeys( bin, &citekeys );
	if ( status!=BIBL_OK ) goto out;

	status = identify_and_resolve_duplicate_citekeys( bin, &citekeys );
out:
	slist_free( &citekeys );
	return status;
}

static int
clean_refs( bibl *bin, param *p )
{
	if ( p->cleanf ) return p->cleanf( bin, p );
	else return BIBL_OK;
}

static int 
convert_refs( bibl *bin, char *fname, bibl *bout, param *p )
{
	int reftype = 0, status;
	fields *rin, *rout;
	long i;

	for ( i=0; i<bin->n; ++i ) {

		rin = bin->ref[i];

		rout = fields_new();
		if ( !rout ) return BIBL_ERR_MEMERR;

		if ( p->typef ) reftype = p->typef( rin, fname, i+1, p );

		status = p->convertf( rin, rout, reftype, p );
		if ( status!=BIBL_OK ) return status;

		if ( p->all ) {
			status = process_alwaysadd( rout, reftype, p );
			if ( status!=BIBL_OK ) return status;
			status = process_defaultadd( rout, reftype, p );
			if ( status!=BIBL_OK ) return status;
		}

		status = bibl_addref( bout, rout );
		if ( status!=BIBL_OK ) return status;
	}

	return BIBL_OK;
}

int
bibl_read( bibl *b, FILE *fp, char *filename, param *p )
{
	// Patch: Disable output logging
	int status = BIBL_OK;
	param read_params;
	bibl bin;

	if ( !b )  return BIBL_ERR_BADINPUT;
	if ( !fp ) return BIBL_ERR_BADINPUT;
	if ( !p )  return BIBL_ERR_BADINPUT;

	if ( bibl_illegalinmode( p->readformat ) ) {
		return BIBL_ERR_BADINPUT;
	}

	status = bibl_setreadparams( &read_params, p );
	if ( status!=BIBL_OK ) {
		return status;
	}

	bibl_init( &bin );

	status = read_refs( fp, &bin, filename, &read_params );
	if ( status!=BIBL_OK ) {
		bibl_freeparams( &read_params );
		return status;
	}

	if ( debug_set( &read_params ) ) {
		bibl_verbose( &bin, "raw_input", "for bibl_read" );
	}

	if ( !read_params.output_raw ) {
		status = clean_refs( &bin, &read_params );
		if ( status!=BIBL_OK ) goto out;
		if ( debug_set( &read_params ) ) bibl_verbose( &bin, "post_clean_refs", "for bibl_read" );
	}

	if ( ( !read_params.output_raw ) || ( read_params.output_raw & BIBL_RAW_WITHCHARCONVERT ) ) {
		status = bibl_fixcharsets( &bin, &read_params );
		if ( status!=BIBL_OK ) goto out;
		if ( debug_set( &read_params ) ) bibl_verbose( &bin, "post_fixcharsets", "for bibl_read" );
	}

	if ( !read_params.output_raw ) {
		status = convert_refs( &bin, filename, b, &read_params );
		if ( status!=BIBL_OK ) goto out;
		if ( debug_set( &read_params ) ) bibl_verbose( b, "post_convert_refs", "for bibl_read" );
	}

	else {
		status = bibl_copy( b, &bin );
		if ( status!=BIBL_OK ) goto out;
		if ( debug_set( &read_params ) ) bibl_verbose( b, "post_bibl_copy", "for bibl_read" );
	}

	if ( ( !read_params.output_raw ) || ( read_params.output_raw & BIBL_RAW_WITHMAKEREFID ) ) {
		status = uniqueify_citekeys( b );
		if ( status!=BIBL_OK ) goto out;
		if ( read_params.addcount ) {
			status = bibl_addcount( b );
			if ( status!=BIBL_OK ) goto out;
		}
		if ( debug_set( &read_params ) ) bibl_verbose( &bin, "post_uniqueify_citekeys", "for bibl_read" );
	}

out:
	bibl_free( &bin );
	bibl_freeparams( &read_params );

	return status;
}

static FILE *
singlerefname( fields *reffields, long nref, int mode )
{
	char outfile[2048];
	char suffix[5] = "xml";
	FILE *fp;
	long count;
	int  found;
	if      ( mode==BIBL_ADSABSOUT )     strcpy( suffix, "ads" );
	else if ( mode==BIBL_BIBTEXOUT )     strcpy( suffix, "bib" );
	else if ( mode==BIBL_ENDNOTEOUT )    strcpy( suffix, "end" );
	else if ( mode==BIBL_ISIOUT )        strcpy( suffix, "isi" );
	else if ( mode==BIBL_MODSOUT )       strcpy( suffix, "xml" );
	else if ( mode==BIBL_RISOUT )        strcpy( suffix, "ris" );
	else if ( mode==BIBL_WORD2007OUT )   strcpy( suffix, "xml" );
	found = fields_find( reffields, "REFNUM", LEVEL_MAIN );
	/* find new filename based on reference */
	if ( found!=-1 ) {
		sprintf( outfile,"%s.%s",(char*)fields_value(reffields,found,FIELDS_CHRP_NOUSE), suffix );
	} else  sprintf( outfile,"%ld.%s",nref, suffix );
	count = 0;
	fp = fopen( outfile, "r" );
	while ( fp ) {
		fclose(fp);
		count++;
		if ( count==60000 ) return NULL;
		if ( found!=-1 )
			sprintf( outfile, "%s_%ld.%s", (char*)fields_value( reffields, found, FIELDS_CHRP_NOUSE ), count, suffix );
		else sprintf( outfile,"%ld_%ld.%s", nref, count, suffix );
		fp = fopen( outfile, "r" );
	}
	return fopen( outfile, "w" );
}

static int
bibl_writeeachfp( FILE *fp, bibl *b, param *p )
{
	fields out, *use = &out;
	int status;
	long i;

	fields_init( &out );

	for ( i=0; i<b->n; ++i ) {

		fp = singlerefname( b->ref[i], i, p->writeformat );
		if ( !fp ) return BIBL_ERR_CANTOPEN;

		if ( p->headerf ) p->headerf( fp, p );

		if ( p->assemblef ) {
			fields_free( &out );
			status = p->assemblef( b->ref[i], &out, p, i );
			if ( status!=BIBL_OK ) break;
		} else {
			use = b->ref[i];
		}

		status = p->writef( use, fp, p, i );

		if ( p->footerf ) p->footerf( fp );
		fclose( fp );

		if ( status!=BIBL_OK ) return status;
	}

	return BIBL_OK;
}

static int
bibl_writefp( FILE *fp, bibl *b, param *p )
{
	// Patch: Disable output logging
	int status = BIBL_OK;
	fields out, *use = &out;
	long i;

	fields_init( &out );

	if ( p->headerf ) p->headerf( fp, p );
	for ( i=0; i<b->n; ++i ) {

		if ( p->assemblef ) {
			fields_free( &out );
			status = p->assemblef( b->ref[i], &out, p, i );
			if ( status!=BIBL_OK ) break;
			if ( debug_set( p ) ) bibl_verbose_reference( &out, "", i+1 );
		} else {
			use = b->ref[i];
		}

		status = p->writef( use, fp, p, i );
		if ( status!=BIBL_OK ) break;

	}

	if ( p->footerf ) p->footerf( fp );
	return status;
}

int
bibl_write( bibl *b, FILE *fp, param *p )
{
	// Patch: Disable output logging
	int status;
	param lp;

	if ( !b ) return BIBL_ERR_BADINPUT;
	if ( !p ) return BIBL_ERR_BADINPUT;
	if ( bibl_illegaloutmode( p->writeformat ) ) return BIBL_ERR_BADINPUT;
	if ( !fp && !p->singlerefperfile ) return BIBL_ERR_BADINPUT;

	status = bibl_setwriteparams( &lp, p );
	if ( status!=BIBL_OK ) return status;

	if ( debug_set( p ) ) bibl_verbose( b, "raw_input", "for bibl_write" );

	status = bibl_fixcharsets( b, &lp );
	if ( status!=BIBL_OK ) goto out;

	if ( debug_set( p ) ) bibl_verbose( b, "post-fixcharsets", "for bibl_write" );

	if ( p->singlerefperfile ) status = bibl_writeeachfp( fp, b, &lp );
	else status = bibl_writefp( fp, b, &lp );

out:
	bibl_freeparams( &lp );
	return status;
}
