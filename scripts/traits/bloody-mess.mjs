export var name = "bloody-mess";

export function onToggled(characterSheet, toggled) {
  if (toggled)
    characterSheet.traits.push(name);
  else
    characterSheet.traits.splice(characterSheet.traits.indexOf(name), 1);
}
