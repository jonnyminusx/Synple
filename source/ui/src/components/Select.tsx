import { useCallback, useState } from 'react'
import { useJuceComboBox } from '../hooks/useJuceComboBox.js'
import { useMidiLearn } from '../hooks/useMidiLearn.js'
import { ContextMenu } from './ContextMenu.js'

interface SelectProps {
  parameterId: string
  label: string
  compact?: boolean
}

export function Select({ parameterId, label, compact = false }: SelectProps) {
  const { choiceIndex, choices, setChoiceIndex } = useJuceComboBox(parameterId)
  const { cc, isLearning, startLearn, cancelLearn, clearLearn } = useMidiLearn(parameterId)
  const [menu, setMenu] = useState<{ x: number; y: number } | null>(null)

  const handleContextMenu = useCallback((e: React.MouseEvent) => {
    e.preventDefault()
    setMenu({ x: e.clientX, y: e.clientY })
  }, [])

  const menuItems = isLearning
    ? [
        { label: 'Cancel Learning', action: cancelLearn, variant: 'danger' as const },
        ...(cc !== null ? [{ label: `Clear CC ${cc}`, action: clearLearn, variant: 'danger' as const }] : []),
      ]
    : [
        { label: 'Learn MIDI CC', action: startLearn },
        ...(cc !== null ? [{ label: `Clear CC ${cc}`, action: clearLearn, variant: 'danger' as const }] : []),
      ]

  return (
    <div
      className="flex flex-col items-center gap-0.5 select-none"
      onContextMenu={handleContextMenu}
    >
      <select
        value={choiceIndex}
        onChange={(e) => setChoiceIndex(Number(e.target.value))}
        className={`bg-zinc-700 border text-zinc-200 text-[10px] font-mono rounded px-1 py-0.5 cursor-pointer focus:outline-none ${
          isLearning
            ? 'border-amber-500'
            : cc !== null
              ? 'border-amber-700'
              : 'border-zinc-600 focus:border-amber-500'
        }${compact ? '' : ' min-w-[80px]'}`}
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
      {isLearning && (
        <span className="text-[8px] font-mono text-amber-500 leading-none animate-pulse">learn</span>
      )}
      {!isLearning && cc !== null && (
        <span className="text-[8px] font-mono text-amber-600 leading-none">cc:{cc}</span>
      )}
      {menu && (
        <ContextMenu
          x={menu.x}
          y={menu.y}
          items={menuItems}
          onClose={() => setMenu(null)}
        />
      )}
    </div>
  )
}
