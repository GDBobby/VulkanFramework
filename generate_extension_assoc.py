import os
import re
import requests
import xml.etree.ElementTree as ET

# ---------------- Configuration ----------------
VK_XML_URL = "https://raw.githubusercontent.com/KhronosGroup/Vulkan-Docs/main/xml/vk.xml"
LOCAL_XML = "vk.xml"
OUTPUT_FILE = "src/ExtensionAssociation.cpp"

# ---------------- Download vk.xml if needed ----------------
if not os.path.exists(LOCAL_XML):
    print("Downloading vk.xml...")
    response = requests.get(VK_XML_URL)
    response.raise_for_status()
    with open(LOCAL_XML, "w", encoding="utf-8") as f:
        f.write(response.text)
else:
    print("Using local vk.xml...")

# ---------------- Parse XML ----------------
tree = ET.parse(LOCAL_XML)
root = tree.getroot()

# ---------------- Parse a single extension ----------------
def parse_extension(ext):
    ext_name = ext.get("name")
    supported = ext.get("supported", "vulkan")
    promoted_to = ext.get("promotedto")

    if "extension_" in ext_name or supported == "disabled":
        return None

    feature_stype = "VK_STRUCTURE_TYPE_MAX_ENUM"
    property_stype = "VK_STRUCTURE_TYPE_MAX_ENUM"

    for require in ext.findall("require"):
        # ---------- Feature struct ----------
        feature_elem = require.find("feature")
        feature_struct = None
        feature_enum = None
        if feature_elem is not None:
            feature_struct = feature_elem.get("struct")
            for enum in require.findall("enum"):
                if enum.get("extends") == "VkStructureType":
                    feature_enum = enum.get("name")
                    feature_stype = enum.get("name")
                    break

        # ---------- Property struct ----------
        property_struct = None
        if feature_elem is not None:
            for type_elem in require.findall("type"):
                type_name = type_elem.get("name")
                if type_name != feature_struct:
                    property_struct = type_name
                    for enum in require.findall("enum"):
                        if enum.get("extends") == "VkStructureType" and enum.get("name") != feature_enum:
                            property_stype = enum.get("name")
                            break
                    break

        if feature_stype != "VK_STRUCTURE_TYPE_MAX_ENUM":
            break

    # ---------- Dependencies ----------
    depends_attr = ext.get("depends", "")
    dependencies = []
    if depends_attr:
        cleaned = depends_attr.replace("(", "").replace(")", "").replace(" ", "")
        parts = re.split(r"[+,]", cleaned)
        for p in parts:
            if re.match(r"^VK_(KHR|EXT|NV|AMD|INTEL|QCOM|GOOGLE|FUCHSIA|MSFT|IMG|ARM|HUAWEI|VALVE)_", p):
                dependencies.append(f'"{p}"')

    # ---------- Promotion version ----------
    promotion_version = ""
    if promoted_to and promoted_to.startswith("VK_VERSION_"):
        m = re.match(r"VK_VERSION_(\d+)_(\d+)", promoted_to)
        if m:
            major, minor = m.groups()
            promotion_version = f"VK_MAKE_VERSION({major}, {minor}, 0)"

    return {
        "name": f'"{ext_name}"',
        "feature_stype": feature_stype,
        "property_stype": property_stype,
        "dependencies": dependencies,
        "promotion_version": promotion_version
    }

# ---------------- Parse all extensions ----------------
extensions = [
    parse_extension(ext) for ext in root.findall(".//extensions/extension")
    if parse_extension(ext) is not None
]

# Sort alphabetically
extensions.sort(key=lambda e: e["name"])

# ---------------- Write C++ file ----------------
with open(OUTPUT_FILE, "w") as f:
    f.write("// Auto-generated ExtensionAssociation table\n")
    f.write("#include \"EightWinds/Helpers/ExtensionAssociation.h\"\n")
    f.write("#include <array>")
    f.write("\nnamespace EWE{\n")
    f.write("static constexpr std::array<ExtensionAssociation> extensionAssociations {\n")

    for e in extensions:
        deps = ", ".join(e["dependencies"]) if e["dependencies"] else ""
        f.write("    ExtensionAssociation{\n")
        f.write(f"        {e['name']},\n")
        f.write(f"        {e['feature_stype']},\n")
        f.write(f"        {e['property_stype']},\n")
        f.write(f"        {{ {deps} }}")
        # Only emit promotion version if present
        if e["promotion_version"]:
            f.write(f",\n        {e['promotion_version']}\n")
        else:
            f.write("\n")
        f.write("    },\n")

    f.write("};\n")
    f.write("} //namespace EWE")

print(f"\nFull table ({len(extensions)} extensions) written to {OUTPUT_FILE}")
