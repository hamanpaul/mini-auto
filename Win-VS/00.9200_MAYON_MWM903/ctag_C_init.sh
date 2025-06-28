#!/bin/sh
find $(pwd) -not \( -path $(pwd)/aal-gen2 -prune -o -path $(pwd)/pon-drv-gen2 -prune \) -name '*.c' -o -name '*.h' -o -name '*.cpp' > cscope.files
cscope -Rbkq -i cscope.files
ctags -R --exclude=.svn --exclude=aal-gen2 --exclude=pon-drv-gen2

######ctags usage#####
#ctags
# Ctrl+]      :According to search keyword which cursor position on the string.(fast key :tj)
# Ctrl+t      :Go back to previous view.
# :tselect    :When there find many keywords in tags file,use ":tselect" to choose.
# :tags       :Looking for where is here(fast key :ta)
# :tnext      :Jump to next tag(fast key :tn)
# :tprev      :Jump to previous tag(fast key :tp)

#cscope
# :cs         :Commands help
# Ctrl + \ + c:Find functions calling this function
# Ctrl + \ + d:Find functions called by this function
# Ctrl + \ + e:Find this egrep pattern
# Ctrl + \ + f:Find this file
# Ctrl + \ + g:Find this definition
# Ctrl + \ + i:Find files #including this file
# Ctrl + \ + s:Find this C symbol
# Ctrl + \ + t:Find this text string
# The hart key is depend on .vimrc setting.
# nmap <F1>s :cs find s <C-R>=expand("<cword>")<CR><CR>
# nmap <F1>g :cs find g <C-R>=expand("<cword>")<CR><CR>
# nmap <F1>c :cs find c <C-R>=expand("<cword>")<CR><CR>
# nmap <F1>t :cs find t <C-R>=expand("<cword>")<CR><CR>
# nmap <F1>e :cs find e <C-R>=expand("<cword>")<CR><CR>
# nmap <F1>f :cs find f <C-R>=expand("<cfile>")<CR><CR>
# nmap <F1>i :cs find i ^<C-R>=expand("<cfile>")<CR>$<CR>
# nmap <F1>d :cs find d <C-R>=expand("<cword>")<CR><CR>
# let Tlist_Show_One_File = 1 "不同时显示多个文件的tag，只显示当前文件的  
# let Tlist_Exit_OnlyWindow = 1 "如果taglist窗口是最后一个窗口，则退出vim 


# '*' find the string in previous context from here (press n、shift + n to find next/previous.)
# '#' find the string in behind context from hare (press n、shift + n to find next/previous.)
# 'gd' 將此文字串視為 local 變數，找到定義此 local 變數的地方 (如果有的話)
# '%' 找到被配的括號
# '[[' 找到函數的開頭
# ']]' 找到下一個函數的開頭
# :set rnu 顯示相對行數，用於復製
# shift + v (Virtual mode), j or k(上或下) 'y' or 'd' 範圍復製或刪除
# 's': 即 Symbol，以此文字串當識別字，列出專案當中所有參考到此識別字的地方，包含定義和引用
# 'g': 即 Global，以此文字串當作 global 變數或函數的名稱，跳到專案中定義此 global 變數或函數的地方 (這個功能有另一個與 ctags 相同的快速鍵 Ctrl-])
# 'c': 即 Calls，以此文字串當函數名稱，找出所有呼叫到的此函數的函數
# 't': 即 Text，列出專案中所有出現此文字串的地方 (包含註解)
# 'e': 即 Egrep，以此文字串當 regular expression，用 egrep 方式來搜尋
# 'f': 即 File，以此文字串當檔案名稱，開啟此檔案
# 'i': 即 Includes，以此文字串當 header 檔名稱，列出所有 include 此檔案的檔案
# 'd': 即 calleD，以此文字串當函數名稱，列出此函數所呼叫到的函數

# tmux usage:
# $ tmux
# Bind-key = Ctrl + b
# Bind-key v	'split vertical pane(垂直)
# Bind-key s	'split horizontal pane(水平)
# Bind-key hjkl	'change pnae to ←↓↑→(hjkl)
# Bind-key :	'enter command line
# :resize -[L|D|U|R] [offset] 'adjust the pane size of side limit.
	
