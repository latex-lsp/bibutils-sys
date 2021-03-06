fn main() {
    let mut build = cc::Build::new();
    build
        .warnings(false)
        .extra_warnings(false)
        .flag_if_supported("-w")
        .define("_CRT_SECURE_NO_WARNINGS", None)
        .file("vendor/lib/adsout.c")
        .file("vendor/lib/adsout_journals.c")
        .file("vendor/lib/bibcore.c")
        .file("vendor/lib/bibl.c")
        .file("vendor/lib/biblatexin.c")
        .file("vendor/lib/biblatexout.c")
        .file("vendor/lib/bibtexin.c")
        .file("vendor/lib/bibtexout.c")
        .file("vendor/lib/bibtextypes.c")
        .file("vendor/lib/bibutils.c")
        .file("vendor/lib/bltypes.c")
        .file("vendor/lib/bu_auth.c")
        .file("vendor/lib/charsets.c")
        .file("vendor/lib/copacin.c")
        .file("vendor/lib/copactypes.c")
        .file("vendor/lib/ebiin.c")
        .file("vendor/lib/endin.c")
        .file("vendor/lib/endout.c")
        .file("vendor/lib/endtypes.c")
        .file("vendor/lib/endxmlin.c")
        .file("vendor/lib/entities.c")
        .file("vendor/lib/fields.c")
        .file("vendor/lib/gb18030.c")
        .file("vendor/lib/generic.c")
        .file("vendor/lib/intlist.c")
        .file("vendor/lib/is_ws.c")
        .file("vendor/lib/isiin.c")
        .file("vendor/lib/isiout.c")
        .file("vendor/lib/isitypes.c")
        .file("vendor/lib/iso639_1.c")
        .file("vendor/lib/iso639_2.c")
        .file("vendor/lib/iso639_3.c")
        .file("vendor/lib/latex.c")
        .file("vendor/lib/latex_parse.c")
        .file("vendor/lib/marc_auth.c")
        .file("vendor/lib/medin.c")
        .file("vendor/lib/modsin.c")
        .file("vendor/lib/modsout.c")
        .file("vendor/lib/modstypes.c")
        // doi.c and doi.h seem to be missing in the source tarball
        // .file("vendor/lib/mycvout.c")
        .file("vendor/lib/name.c")
        .file("vendor/lib/nbibin.c")
        .file("vendor/lib/nbibout.c")
        .file("vendor/lib/nbibtypes.c")
        .file("vendor/lib/notes.c")
        .file("vendor/lib/pages.c")
        .file("vendor/lib/reftypes.c")
        .file("vendor/lib/risin.c")
        .file("vendor/lib/risout.c")
        .file("vendor/lib/ristypes.c")
        .file("vendor/lib/serialno.c")
        .file("vendor/lib/slist.c")
        .file("vendor/lib/str.c")
        .file("vendor/lib/str_conv.c")
        .file("vendor/lib/strsearch.c")
        .file("vendor/lib/title.c")
        .file("vendor/lib/type.c")
        .file("vendor/lib/unicode.c")
        .file("vendor/lib/url.c")
        .file("vendor/lib/utf8.c")
        .file("vendor/lib/vplist.c")
        .file("vendor/lib/wordin.c")
        .file("vendor/lib/wordout.c")
        .file("vendor/lib/xml.c")
        .file("vendor/lib/xml_encoding.c");

    if cfg!(windows) {
        build
            .define("strcasecmp", "_stricmp")
            .define("strncasecmp", "_strnicmp");
    }

    build.compile("bibutils");
}
