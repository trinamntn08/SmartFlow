import { useEffect, useState } from 'react';
import type { JsonValue } from '@smartflow/core';
import type { ParameterDefinition } from '@smartflow/extension-sdk';
export function ParameterEditor({
  parameter,
  value,
  commit,
}: {
  parameter: ParameterDefinition;
  value: JsonValue;
  commit(value: JsonValue): void;
}) {
  const [draft, setDraft] = useState(String(value));
  useEffect(() => {
    setDraft(String(value));
  }, [value]);
  if (parameter.control === 'boolean')
    return (
      <label className="parameter">
        <span>{parameter.label}</span>
        <input
          type="checkbox"
          checked={value === true}
          onChange={(event) => commit(event.target.checked)}
        />
      </label>
    );
  if (parameter.control === 'select')
    return (
      <label className="parameter">
        <span>{parameter.label}</span>
        <select
          value={JSON.stringify(value)}
          onChange={(event) => commit(JSON.parse(event.target.value))}
        >
          {parameter.options?.map((option) => (
            <option key={JSON.stringify(option)} value={JSON.stringify(option)}>
              {String(option)}
            </option>
          ))}
        </select>
      </label>
    );
  const save = () => {
    if (draft !== String(value)) {
      commit(parameter.control === 'number' ? (draft.trim() ? Number(draft) : null) : draft);
    }
  };
  return (
    <label className="parameter">
      <span>{parameter.label}</span>
      <input
        aria-label={parameter.label}
        type={parameter.control === 'number' ? 'number' : 'text'}
        value={draft}
        min={parameter.minimum}
        max={parameter.maximum}
        step="any"
        onChange={(event) => setDraft(event.target.value)}
        onBlur={save}
        onKeyDown={(event) => {
          if (event.key === 'Enter') event.currentTarget.blur();
          if (event.key === 'Escape') {
            setDraft(String(value));
            event.currentTarget.blur();
          }
        }}
      />
    </label>
  );
}
