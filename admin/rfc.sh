#!/bin/bash


function usage()
{
    less -e <<"EOF"
Script to maintain Lumiera RFC's

usage:
 ./admin/rfc.sh <command> [options]

options:
 title                  - Quoted string used as RFC title
 rfc                    - Name of the RFC, smart matched
 regex                  - Regex matched against the content of a RFC
 chapter                - Heading of a section

commands (with <mandatory> and [optional] parameters):
 help                   - Show this help
 find <rfc> [regex]     - List all matching RFC's (matching 'regex')
 show <rfc> [regex]     - Read RFC's (matching 'regex')
 edit <rfc> [chapter]   - Edit RFC at chapter
 process                - Do automatic maintenance work
 create <title>         - Create a new RFC
 draft <rfc>            - Change RFC to Draft state
 park <rfc>             - Change RFC to Parked state
 final <rfc>            - Change RFC to Final state
 drop <rfc>             - Change RFC to Dropped state
 comment <rfc>          - Add a new comment to a RFC
 discard <rfc>          - Delete an RFC

Smart matching:
 RFC names don't need to be given exactly, they use a globbing pattern.
 This is:
  - case insensitive
  - whitespaces are ignored
  - '*' stands for any number of parameters
  - '?' is any single character
  - when starting with '/' they are matched against the begin of the name
  - some regex operators work too
 'find' and 'show' can operate on many matches so the given rfc name doesn't
 need to be unique. The other commands will complain when the RFC name given
 doesn't resolve to one unique RFC.

Notes:
 When less presents multiple files one can go forth and back with the ':n'
 and ':p' commands.

 The tile for 'create' should be a normal document title. Possibly quoted
 since it may contain spaces and not too long. The filename is this title
 in CamelCase with all spaces and special characters removed.

 Chapter machching single lines containing this word, special asciidoc
 comments in the form '//word:.*' and asciidoc block attributes '[word.*]'
 on a single line. When a chapter pattern is not unique, the last one is
 picked.

 rfc.sh executes git add/rm/mv commands, but never commits. One should do a
 commit as soon he finished editing.

EOF
}


function camel_case()
{
    local c
    local u
    local t=" $1"

    for c in {a..z}; do
        u=$(tr "a-z" "A-Z" <<<"$c")
        t="${t// $c/$u}"
    done

    echo "$(tr -c -d "A-Za-z" <<<"$t")"
}


function find_rfc()
{
    # find rfc by shortname
    local file
    local match="$1"
    match="${match// /}"
    match="${match//./\.}"
    match="${match//\?/.}"
    match="${match//\*/.*}"

    local globstate=$(shopt -p nocasematch)
    shopt -s nocasematch

    for file in $(find ./doc/devel/rfc* -name '*.txt');
    do
        local name="/${file##*/}"
        if [[ "$name" =~ $match ]]; then
            echo "$file"
        fi
    done
    $globstate
}



function find_chapter()
{
    # find chapter
    local file="$1"
    local chapter="$2"
    local found=$(grep -n -i "^\($chapter\|//$chapter:.*\|\[$chapter.*\]\)\$" "$file" | tail -1)
    if [[ "$found" ]]; then
        echo "${found%%:*}"
    fi
}



function process()
{
    local file="$1"
    local path="${1%/*}"
    local state=$(grep '^\*State\* *' "$file")

    case "$state" in
    *Final*)
        if [[ "$path" != "./doc/devel/rfc" ]]; then
            git mv "$file" "./doc/devel/rfc"
        fi
        ;;
    *Idea*|*Draft*)
        if [[ "$path" != "./doc/devel/rfc_pending" ]]; then
            git mv "$file" "./doc/devel/rfc_pending"
        fi
        ;;
    *Parked*|*Dropped*)
        if [[ "$path" != "./doc/devel/rfc_dropped" ]]; then
            git mv "$file" "./doc/devel/rfc_dropped"
        fi
        ;;
    esac
}


function edit()
{
    # filename lineoffset chapter
    EDITOR="${EDITOR:-$(git config --get core.editor)}"
    EDITOR="${EDITOR:-VISUAL}"

    local file="$1"
    local line=0

    if [[ "$3" ]]; then
        line=$(find_chapter "$file" "$3")
    fi

    $EDITOR +$(($line+${2:-1})) $file
}


function unique_name()
{
    local files=($(find_rfc "$1"))

    if [[ ${#files[*]} -gt 1 ]]; then
        echo "multiple matches:" >&2
        (
            IFS=$'\n'
            echo "${files[*]}" >&2
        )
    elif [[ ${#files[*]} -eq 0 ]]; then
        echo "no matches" >&2
    else
        echo ${files[0]}
    fi
}


function add_comment()
{
    local name="$1"
    local nl=$'\n'
    local comment="//edit comment$nl    $(date +%c) $(git config --get user.name) <$(git config --get user.email)>$nl"

    ed "$name" 2>/dev/null <<EOF
/endof_comments:/-1i
$comment
.
wq
EOF

    edit "$name" -4 "endof_comments"
}

function edit_state()
{
    local name="$1"
    local state="$2"
    local comment="$3"

    ed "$name" 2>/dev/null <<EOF
/^\*State\*/s/\(\*State\* *_\).*_/\1${state}_/
wq
EOF

    if [[ "$comment" ]]; then

        ed "$name" 2>/dev/null <<EOF
/endof_comments:/-1i
$comment
.
wq
EOF

    fi
}


function change_state()
{
    #rfcname state
    local name="$1"
    local state="$2"

    local nl=$'\n'
    local comment="$state$nl//add reason$nl    $(date +%c) $(git config --get user.name) <$(git config --get user.email)>$nl"
    edit_state "$name" "$state" "$comment"
    edit "$name" -4 "endof_comments"
    process "$name"
}


command="$1"
shift

case "$command" in
process)
    # for all rfc's
    for file in $(find ./doc/devel/rfc* -name '*.txt');
    do
        process "$file"
    done
    :
    ;;
find|list|ls)
    if [[ "$2" ]]; then
        find_rfc "$1" | xargs grep -i -C3 -n "$2"
    else
        find_rfc "$1"
    fi
    ;;
show|less|more)
    if [[ "$2" ]]; then
        less $(find_rfc "$1" | xargs grep -i -l "$2")
    else
        less $(find_rfc "$1")
    fi
    ;;
create)
    TITLE="$@"
    name=$(camel_case "$TITLE")
    if [[ -f "./doc/devel/rfc/${name}.txt" ||
          -f "./doc/devel/rfc_pending/${name}.txt" ||
          -f "./doc/devel/rfc_dropped/${name}.txt" ]]; then
        echo "$name.txt exists already"
    else
        source ./doc/template/rfc.txt >"./doc/devel/rfc_pending/${name}.txt"
        edit "./doc/devel/rfc_pending/${name}.txt" 2 abstract
        git add "./doc/devel/rfc_pending/${name}.txt"
        process "./doc/devel/rfc_pending/${name}.txt"
    fi
    ;;
edit)
    name=$(unique_name "$1")
    if [[ "$name" ]]; then
        edit "${name}" 2 "$2"
    fi
    ;;
draft)
    name=$(unique_name "$1")
    if [[ "$name" ]]; then
        change_state "$name" Draft
    fi
    ;;
park)
    name=$(unique_name "$1")
    if [[ "$name" ]]; then
        change_state "$name" Parked
    fi
    ;;
final)
    name=$(unique_name "$1")
    if [[ "$name" ]]; then
        change_state "$name" Final
    fi
    ;;
drop)
    name=$(unique_name "$1")
    if [[ "$name" ]]; then
        change_state "$name" Dropped
    fi
    ;;
comment)
    name=$(unique_name "$1")
    if [[ "$name" ]]; then
        add_comment "${name}"
    fi
    ;;
discard)
    name=$(unique_name "$1")
    if [[ "$name" ]]; then
        git rm "${name}"
    fi
    ;;
help|*)
    usage
    ;;
esac



