#[allow(warnings)]
mod ffi;

pub use self::ffi::*;

#[cfg(test)]
mod tests {
    use super::{
        bibl_free, bibl_freeparams, bibl_init, bibl_initparams, bibl_read, bibl_write, fopen,
        BIBL_BIBTEXIN, BIBL_OK, BIBL_RISOUT,
    };
    use indoc::indoc;
    use std::{ffi::CString, fs, mem::MaybeUninit};
    use tempfile::tempdir;

    #[test]
    fn bibtex_to_ris() {
        unsafe {
            let mut context = MaybeUninit::zeroed();
            bibl_init(context.as_mut_ptr());

            let mut params = MaybeUninit::zeroed();
            let program = CString::new("bibutils-sys").unwrap();
            bibl_initparams(
                params.as_mut_ptr(),
                BIBL_BIBTEXIN as i32,
                BIBL_RISOUT as i32,
                program.as_ptr() as *mut i8,
            );

            let dir = tempdir().unwrap();
            let input_path = dir.path().join("references.bib");
            fs::write(
                &input_path,
                indoc!(
                    r#"
                        @article{foo,
                            author = {Foo Bar},
                            title = {Baz Qux},
                            year = 2020
                        }
                    "#
                ),
            )
            .unwrap();

            let input_path = CString::new(input_path.to_str().unwrap()).unwrap();
            let input_mode = CString::new("r").unwrap();
            let input_handle = fopen(input_path.as_ptr(), input_mode.as_ptr());
            let status = bibl_read(
                context.as_mut_ptr(),
                input_handle,
                input_path.as_ptr() as *mut i8,
                params.as_mut_ptr(),
            );
            assert_eq!(status, BIBL_OK as i32);

            let output_path = dir.path().join("references.ris");
            let output_path = CString::new(output_path.to_str().unwrap()).unwrap();
            let output_mode = CString::new("w").unwrap();
            let output_handle = fopen(output_path.as_ptr(), output_mode.as_ptr());
            let status = bibl_write(context.as_mut_ptr(), output_handle, params.as_mut_ptr());
            assert_eq!(status, BIBL_OK as i32);

            let data = fs::read(dir.path().join("references.ris")).unwrap();
            let text = String::from_utf8_lossy(&data[3..]).into_owned();

            bibl_freeparams(params.as_mut_ptr());
            bibl_free(context.as_mut_ptr());

            println!("{}", text);
            assert_eq!(
                text.trim().replace("\r\n", "\n"),
                indoc!(
                    r#"
                        TY  - JOUR
                        AU  - Bar, Foo
                        PY  - 2020
                        DA  - 2020//
                        TI  - Baz Qux
                        ID  - foo
                        ER  -
                    "#
                )
                .trim()
            );
        }
    }
}
