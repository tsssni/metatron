vim.lsp.config('slangd', {
  settings = {
    slang = {
      additionalSearchPaths = { "./shader/" },
      searchInAllWorkspaceDirectories = true,
      inlayHints = {
        deducedTypes = false,
        parameterNames = false,
      }
    }
  },
  root_dir = './shader/'
})
