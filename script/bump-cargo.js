const projectVersionRegExp = /(?<name>name = "swappy"\s*)version = "(?<version>\d+\.\d+\.\d+)"/;

module.exports.readVersion = function (contents) {
  const matches = contents.match(projectVersionRegExp);

  return matches ? matches[1] : "unknown";
};

module.exports.writeVersion = function (_contents, version) {
  return _contents.replace(projectVersionRegExp, `$1version = "${version}"`);
};
