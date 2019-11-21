
try:
    import artifactory_utils
except:
    pass
else:
    dependencies = [
        artifactory_utils.ArtifactSelector(
            project="Toolchain-Release",
            revision="ReleaseMorepork1.5",
            version="^4.6",
            stable_required=True)
    ]
