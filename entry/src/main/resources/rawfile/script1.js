{
    let a = "hello World";
    const mPromise = createPromise();
    mPromise.then((result) => {
        assertEqual(result, 0);
        onJSResultCallback(result, "abc", "v");
    });
    b.toUpperCase();
};