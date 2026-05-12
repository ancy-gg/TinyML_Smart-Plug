local stringify = pandoc.utils.stringify

local rendered_labels = {
  "fig:ch3-pwa-block",
  "fig:ch3-pwa-dashboard",
  "fig:ch3-pwa-history",
  "fig:ch3-full-schematic",
  "fig:ch3-data-labeling-deploy",
  "fig:ch3-tinyml-sequence",
  "fig:ch3-voltage-regression",
  "fig:ch3-current-regression",
  "fig:ch3-ntc-cal-setup",
  "fig:ch3-overload-block",
  "fig:ch3-heating-block",
  "fig:ch3-arc-block",
  "fig:ch4-char-water-heater",
  "fig:ch4-char-one-lamp",
  "fig:ch4-char-two-lamps",
  "fig:ch4-char-three-lamps",
  "fig:ch4-char-four-lamps",
  "fig:ch4-protect-water-heater",
  "fig:ch4-protect-one-lamp",
  "fig:ch4-protect-two-lamps",
  "fig:ch4-protect-three-lamps",
  "fig:ch4-protect-four-lamps",
  "fig:ch4-group-metrics",
  "fig:ch4-setup-outcomes",
  "fig:ch4-family-outcomes",
  "fig:app-h-gantt-chart",
}

local rendered = {}
for _, label in ipairs(rendered_labels) do
  local name = label:gsub("[^A-Za-z0-9]+", "_"):gsub("^_+", ""):gsub("_+$", "")
  rendered[label] = "word/rendered_figures/" .. name .. ".png"
end

local aux_files = {
  "logs/chapters/chapter_1/main.aux",
  "logs/chapters/chapter_2/main.aux",
  "logs/chapters/chapter_3/main.aux",
  "logs/chapters/chapter_4/main.aux",
  "logs/chapters/chapter_5/main.aux",
  "logs/appendices/appendix_a.aux",
  "logs/appendices/appendix_b.aux",
  "logs/appendices/appendix_c.aux",
  "logs/appendices/appendix_d.aux",
  "logs/appendices/appendix_e.aux",
  "logs/appendices/appendix_f.aux",
  "logs/appendices/appendix_g.aux",
  "logs/appendices/appendix_h.aux",
  "logs/appendices/appendix_i.aux",
}

local label_numbers = {}
for _, path in ipairs(aux_files) do
  local file = io.open(path, "r")
  if file then
    for line in file:lines() do
      local id, number = line:match("\\newlabel{([^}]+)}{{([^}]+)}")
      if id and number then
        label_numbers[id] = number
      end
    end
    file:close()
  end
end

local function trim(s)
  return (s:gsub("^%s+", ""):gsub("%s+$", ""))
end

local function has_class(el, class)
  for _, value in ipairs(el.classes or {}) do
    if value == class then
      return true
    end
  end
  return false
end

local function raw_openxml(xml)
  return pandoc.RawBlock("openxml", xml)
end

local function page_break()
  return raw_openxml('<w:p><w:r><w:br w:type="page"/></w:r></w:p>')
end

local function toc_blocks()
  return {
    page_break(),
    raw_openxml('<w:p><w:pPr><w:pStyle w:val="TOCHeading"/></w:pPr><w:r><w:t>TABLE OF CONTENTS</w:t></w:r></w:p>'),
    raw_openxml('<w:p><w:r><w:fldChar w:fldCharType="begin" w:dirty="true"/></w:r><w:r><w:instrText xml:space="preserve"> TOC \\o "1-3" \\h \\z \\u </w:instrText></w:r><w:r><w:fldChar w:fldCharType="separate"/></w:r><w:r><w:t>Right-click and update field.</w:t></w:r><w:r><w:fldChar w:fldCharType="end"/></w:r></w:p>'),
    page_break(),
  }
end

local function append_all(target, values)
  for _, value in ipairs(values) do
    table.insert(target, value)
  end
end

local function prefix_caption(caption, kind, number)
  if not (caption and caption.long and number) then
    return caption
  end

  if #caption.long == 0 then
    caption.long = { pandoc.Plain{ pandoc.Str(kind), pandoc.Space(), pandoc.Str(number .. ".") } }
    return caption
  end

  local first = caption.long[1]
  if first.t ~= "Plain" and first.t ~= "Para" then
    table.insert(caption.long, 1, pandoc.Plain{ pandoc.Str(kind), pandoc.Space(), pandoc.Str(number .. ".") })
    return caption
  end

  local text = trim(stringify(first))
  if text:match("^" .. kind .. "%s+" .. number:gsub("%.", "%%.")) then
    return caption
  end

  local prefix = { pandoc.Str(kind), pandoc.Space(), pandoc.Str(number .. "."), pandoc.Space() }
  for i = #prefix, 1, -1 do
    table.insert(first.content, 1, prefix[i])
  end
  return caption
end

local function bibliography_div(block)
  return pandoc.Div({ block }, pandoc.Attr("", {}, { ["custom-style"] = "Bibliography" }))
end

function Math(el)
  local text = el.text
  for _, env in ipairs({ "equation", "align", "aligned", "gather", "multline" }) do
    text = text:gsub("\\begin%s*{%s*" .. env .. "%*?%s*}", "")
    text = text:gsub("\\end%s*{%s*" .. env .. "%*?%s*}", "")
  end
  text = text:gsub("\\label%s*{[^}]*}", "")
  text = text:gsub("\\notag", "")
  text = text:gsub("\\degreeC", "^{\\circ}\\mathrm{C}")

  local changed
  repeat
    text, changed = text:gsub("\\ensuremath%s*{([^{}]*)}", "%1")
  until changed == 0

  el.text = trim(text)
  return el
end

function Para(el)
  local text = trim(stringify(el))
  if text == "" or text == "1.15" or text == "1.5" then
    return {}
  end
  return el
end

function Div(el)
  if has_class(el, "spacing") and #el.content > 0 then
    local first = el.content[1]
    if first.t == "Para" and #first.content > 0 then
      local marker = trim(stringify(first.content[1]))
      if marker:match("^%d+%.?%d*$") then
        table.remove(first.content, 1)
        if first.content[1] and first.content[1].t == "SoftBreak" then
          table.remove(first.content, 1)
        end
        if #first.content == 0 then
          table.remove(el.content, 1)
        end
      end
    end
  end

  if trim(stringify(el)) == "" then
    return {}
  end

  local number = label_numbers[el.identifier]
  if number and el.identifier:match("^tab:") then
    for _, block in ipairs(el.content) do
      if block.t == "Table" then
        block.caption = prefix_caption(block.caption, "Table", number)
      end
    end
  end

  return el
end

function Figure(el)
  local path = rendered[el.identifier]
  if path then
    local img = pandoc.Image({}, path, "")
    img.attributes.width = "5.75in"
    if el.identifier == "fig:ch3-full-schematic" then
      img.attributes.width = "5.9in"
    end
    el.content = { pandoc.Plain{ img } }
  end

  local number = label_numbers[el.identifier]
  if number then
    el.caption = prefix_caption(el.caption, "Figure", number)
  end
  return el
end

function Table(el)
  local number = label_numbers[el.identifier]
  if number then
    el.caption = prefix_caption(el.caption, "Table", number)
  end
  return el
end

function Pandoc(doc)
  local blocks = {}
  local inserted_toc = false
  local in_references = false

  for _, block in ipairs(doc.blocks) do
    if block.t == "Div" and has_class(block, "titlepage") then
      table.insert(blocks, block)
      table.insert(blocks, page_break())
    elseif block.t == "Header" and trim(stringify(block)) == "INTRODUCTION" and not inserted_toc then
      append_all(blocks, toc_blocks())
      inserted_toc = true
      table.insert(blocks, block)
    elseif block.t == "Header" and trim(stringify(block)) == "REFERENCES" then
      in_references = true
      table.insert(blocks, block)
    elseif in_references and block.t == "Header" and block.level <= 1 then
      in_references = false
      table.insert(blocks, block)
    elseif in_references and (block.t == "Para" or block.t == "Plain") then
      table.insert(blocks, bibliography_div(block))
    elseif in_references and block.t == "Div" then
      block.attributes["custom-style"] = "Bibliography"
      table.insert(blocks, block)
    else
      table.insert(blocks, block)
    end
  end

  doc.blocks = blocks
  return doc
end
