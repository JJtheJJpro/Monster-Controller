use pyo3::Python;

fn main() {
    let pycode = String::from_utf8(include_bytes!("../console.py").to_vec()).unwrap();

    pyo3::prepare_freethreaded_python();

    Python::with_gil(|py| {
        py.run_bound(&pycode, None, None).unwrap();
    });
}
