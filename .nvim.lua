vim.lsp.enable('clangd')
vim.lsp.enable('cmake')
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
vim.lsp.enable('slangd')
vim.lsp.enable('nushell')
