import { useJuceComboBox } from '../hooks/useJuceComboBox.js'

interface SelectProps {
  parameterId: string
  label: string
}

export function Select({ parameterId, label }: SelectProps) {
  const { choiceIndex, choices, setChoiceIndex } = useJuceComboBox(parameterId)

  return (
    <div className="flex flex-col items-center gap-0.5 select-none">
      <select
        value={choiceIndex}
        onChange={(e) => setChoiceIndex(Number(e.target.value))}
        className="bg-zinc-700 border border-zinc-600 text-zinc-200 text-[10px] font-mono rounded px-1 py-0.5 cursor-pointer focus:outline-none focus:border-amber-500 w-full"
      >
        {choices.map((choice, i) => (
          <option key={i} value={i}>
            {choice}
          </option>
        ))}
      </select>
      <span className="text-[9px] text-zinc-800 font-mono uppercase tracking-wide leading-none">
        {label}
      </span>
    </div>
  )
}
