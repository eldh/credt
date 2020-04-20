include Rely.Make({
  let config =
    Rely.TestFrameworkConfig.initialize({
      snapshotDir: "./__snapshots",
      projectDir: "./library"
    });
});